#include <cstring>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tensorflow/detection.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Runs a local server with an endpoint called 'detect_from_camera', which
// will capture an image from the board's camera, run the image through an
// object detection model and return the results in a JSON response.
//
// The response includes only the top result with a JSON file like this:
//
// {
// 'id': int,
// 'result':
//     {
//     'width': int,
//     'height': int,
//     'base64_data': image_bytes,
//     'detection':
//         {
//         'id': int,
//         'score': float,
//         'xmin': float,
//         'xmax': float,
//         'ymin': float,
//         'ymax': float
//         }
//     }
// }

namespace {
constexpr char kModelPath[] =
    "/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite";
// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 8 * 1024 * 1024;
uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)))
__attribute__((section(".sdram_bss,\"aw\",%nobits @")));

void DetectFromCamera(struct jsonrpc_request* r) {
    auto* interpreter =
        reinterpret_cast<tflite::MicroInterpreter*>(r->ctx->response_cb_data);

    auto* input_tensor = interpreter->input_tensor(0);
    int model_height = input_tensor->dims->data[1];
    int model_width = input_tensor->dims->data[2];

    printf("width=%d; height=%d\n\r", model_width, model_height);

    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::STREAMING);

    std::vector<uint8_t> image(model_width * model_height * /*channels=*/3);
    coral::micro::camera::FrameFormat fmt{coral::micro::camera::Format::RGB, coral::micro::camera::FilterMethod::BILINEAR,
                                     coral::micro::camera::Rotation::k0, model_width,
                                     model_height, false, image.data()};

    bool ret = coral::micro::CameraTask::GetFrame({fmt});

    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::CameraTask::GetSingleton()->SetPower(false);

    if (!ret) {
        jsonrpc_return_error(r, -1, "Failed to get image from camera.",
                             nullptr);
        return;
    }

    std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image.data(),
                image.size());

    if (interpreter->Invoke() != kTfLiteOk) {
        jsonrpc_return_error(r, -1, "Invoke failed", nullptr);
        return;
    }

    auto results =
        coral::micro::tensorflow::GetDetectionResults(interpreter, 0.5, 1);
    if (!results.empty()) {
        const auto& result = results[0];
        jsonrpc_return_success(
            r,
            "{%Q: %d, %Q: %d, %Q: %V, %Q: {%Q: %d, %Q: %g, %Q: %g, %Q: %g, "
            "%Q: %g, %Q: %g}}",
            "width", model_width, "height", model_height, "base64_data",
            image.size(), image.data(), "detection", "id", result.id, "score",
            result.score, "xmin", result.bbox.xmin, "xmax", result.bbox.xmax,
            "ymin", result.bbox.ymin, "ymax", result.bbox.ymax);
        return;
    }
    jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: None}", "width",
                           model_width, "height", model_height, "base64_data",
                           image.size(), image.data(), "detection");
}
}  // namespace

extern "C" void app_main(void* param) {
    std::vector<uint8_t> model;
    if (!coral::micro::filesystem::ReadFile(kModelPath, &model)) {
        printf("ERROR: Failed to load %s\r\n", kModelPath);
        vTaskSuspend(nullptr);
    }

    auto tpu_context = coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!tpu_context) {
        printf("ERROR: Failed to get EdgeTpu context\r\n");
        vTaskSuspend(nullptr);
    }

    tflite::MicroErrorReporter error_reporter;
    tflite::MicroMutableOpResolver<3> resolver;
    resolver.AddDequantize();
    resolver.AddDetectionPostprocess();
    resolver.AddCustom(coral::micro::kCustomOp, coral::micro::RegisterCustomOp());

    tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()),
                                         resolver, tensor_arena,
                                         kTensorArenaSize, &error_reporter);
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        printf("ERROR: AllocateTensors() failed\r\n");
        vTaskSuspend(nullptr);
    }

    if (interpreter.inputs().size() != 1) {
        printf("ERROR: Model must have only one input tensor\r\n");
        vTaskSuspend(nullptr);
    }

    printf("Initializing detection server...%p\r\n", &interpreter);
    jsonrpc_init(nullptr, &interpreter);
    jsonrpc_export("detect_from_camera", DetectFromCamera);
    coral::micro::UseHttpServer(new coral::micro::JsonRpcHttpServer);
    printf("Detection server ready!\r\n");
    vTaskSuspend(nullptr);
}
