#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/utils.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tensorflow/detection.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/httpsrv/httpsrv_base64.h"

namespace {
    constexpr int kTensorArenaSize{8 * 1024 * 1024};

    std::unique_ptr<tflite::MicroInterpreter> interpreter{nullptr};
    std::shared_ptr<valiant::EdgeTpuContext> edgetpu_context{nullptr};

    int model_height{0};
    int model_width{0};
    // An area of memory to use for input, output, and intermediate arrays.
    uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

    void detect_from_camera_rpc(struct jsonrpc_request *r) {
        valiant::CameraTask::GetSingleton()->SetPower(true);
        valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);

        if (model_height == 0 || model_width == 0) {
            jsonrpc_return_error(r, -1, "Model height/width not initialized.", nullptr);
            return;
        }
        std::vector<uint8_t> image_buffer(model_width * model_height * /*channels=*/3);
        valiant::camera::FrameFormat fmt{valiant::camera::Format::RGB,
                                         model_width,
                                         model_height,
                                         false,
                                         image_buffer.data()};

        bool ret = valiant::CameraTask::GetFrame({fmt});

        valiant::CameraTask::GetSingleton()->Disable();
        valiant::CameraTask::GetSingleton()->SetPower(false);

        if (!ret) {
            jsonrpc_return_error(r, -1, "Failed to get image from camera.", nullptr);
            return;
        } else {
            // Copy image data to input tensor.
            auto *input_tensor_data = tflite::GetTensorData<uint8_t>(interpreter->input_tensor(0));
            std::memcpy(input_tensor_data, image_buffer.data(), image_buffer.size());

            // Run Inference.
            if (interpreter->Invoke() != kTfLiteOk) {
                jsonrpc_return_error(r, -1, "Invoke failed", nullptr);
                return;
            }

            auto base64_size = valiant::utils::Base64Size(image_buffer.size());
            std::vector<char> base64_data(base64_size);
            base64_encode_binary(reinterpret_cast<char *>(image_buffer.data()), base64_data.data(),
                                 image_buffer.size());

            // Parses output and returns to client.
            auto detection_results = valiant::tensorflow::GetDetectionResults(interpreter.get(), 0.5, 1);
            if (!detection_results.empty()) {
                const auto &result = detection_results[0];
                jsonrpc_return_success(r,
                                       "{%Q: %d, %Q: %d, %Q: %.*Q, %Q: {%Q: %d, %Q: %g, %Q: %g, %Q: %g, %Q: %g, %Q: %g}}",
                                       "width", model_width,
                                       "height", model_height,
                                       "base64_data", base64_data.size(), base64_data.data(),
                                       "detection", "id", result.id,
                                       "score", result.score,
                                       "xmin", result.bbox.xmin,
                                       "xmax", result.bbox.xmax,
                                       "ymin", result.bbox.ymin,
                                       "ymax", result.bbox.ymax);
                return;
            }
            jsonrpc_return_success(r,
                                   "{%Q: %d, %Q: %d, %Q: %.*Q, %Q: None}",
                                   "width", model_width,
                                   "height", model_height,
                                   "base64_data", base64_data.size(), base64_data.data(),
                                   "detection");

        }
    }

} // namespace example

extern "C" void app_main(void *param) {
    printf("Initializing the interpreter...\n");
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);

    size_t model_size;
    std::unique_ptr<uint8_t> model_data(valiant::filesystem::ReadToMemory(
            "/models/ssdlite_mobiledet_coco_qat_postprocess_edgetpu.tflite", &model_size));
    if (!model_data || model_size == 0) {
        printf("Failed to load inference_info %p %d\r\n", model_data.get(), model_size);
        vTaskSuspend(nullptr);
    }
    const auto *model = tflite::GetModel(model_data.get());

    edgetpu_context = valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);
    if (!edgetpu_context) {
        printf("Failed to get TPU context\r\n");
        vTaskSuspend(nullptr);
    }

    // Although only 2 ops are registered here, the resolver needs 3 because MakeEdgeTpuInterpreter
    // is going to register an extra 'edgetpu-custom-op'.
    constexpr int kNumTensorOps{3};
    auto resolver = std::make_unique<tflite::MicroMutableOpResolver<kNumTensorOps>>();
    resolver->AddDequantize();
    resolver->AddDetectionPostprocess();

    auto error_reporter = std::make_shared<tflite::MicroErrorReporter>();
    interpreter =
            valiant::tensorflow::MakeEdgeTpuInterpreter(model, edgetpu_context.get(), resolver.get(),
                                                        error_reporter.get(),
                                                        tensor_arena, kTensorArenaSize);
    if (!interpreter) {
        printf("Failed to make interpreter\r\n");
        vTaskSuspend(nullptr);
    }

    model_height = interpreter->input_tensor(0)->dims->data[1];
    model_width = interpreter->input_tensor(0)->dims->data[2];


    printf("Initializing the server...\n");
    valiant::rpc::RPCServerIOHTTP rpc_server_io_http;
    valiant::rpc::RPCServer rpc_server;
    if (!rpc_server_io_http.Init()) {
        printf("Failed to initialize RPCServerIOHTTP\r\n");
        vTaskSuspend(nullptr);
    }
    if (!rpc_server.Init()) {
        printf("Failed to initialize RPCServer\r\n");
        vTaskSuspend(nullptr);
    }
    rpc_server.RegisterIO(rpc_server_io_http);
    rpc_server.RegisterRPC("detect_from_camera", detect_from_camera_rpc);
    printf("Detection server ready\r\n");
    vTaskSuspend(nullptr);
}