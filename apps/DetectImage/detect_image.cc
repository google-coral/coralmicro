#include "libs/base/filesystem.h"
#include "libs/tensorflow/detection.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tensorflow/tensorflow/lite/schema/schema_generated.h"

#include <cstdio>

namespace valiant {

namespace {
    constexpr int kTensorArenaSize = 8 * 1024 * 1024;
    static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
}  // namespace

void real_main() {
    constexpr tensorflow::ImageDims input_dims = {
        341,
        512,
        3
    };
    size_t model_size, input_size;
    auto model_data = filesystem::ReadToMemory("/models/ssdlite_mobiledet_coco_qat_postprocess_edgetpu.tflite", &model_size);
    if (!model_data || model_size == 0) {
        printf("Failed to load model %p %d\r\n", model_data.get(), model_size);
        return;
    }

    auto input_data = filesystem::ReadToMemory("/apps/DetectImage/cat.rgb", &input_size);
    if (!input_data || input_size == 0) {
        printf("Failed to load input\r\n");
        return;
    }

    const tflite::Model* model = tflite::GetModel(model_data.get());

    std::unique_ptr<tflite::MicroErrorReporter> error_reporter(new tflite::MicroErrorReporter());
    constexpr int kNumOps = 3;
    std::unique_ptr<tflite::MicroMutableOpResolver<kNumOps>> resolver(new tflite::MicroMutableOpResolver<kNumOps>);
    resolver->AddDequantize();
    resolver->AddDetectionPostprocess();
    std::shared_ptr<EdgeTpuContext> context = EdgeTpuManager::GetSingleton()->OpenDevice(PerformanceMode::kMax);
    if (!context) {
        printf("Failed to get TPU context\r\n");
        return;
    }
    std::unique_ptr<tflite::MicroInterpreter> interpreter =
        tensorflow::MakeEdgeTpuInterpreter(model, context.get(),
        resolver.get(), error_reporter.get(),
        tensor_arena, kTensorArenaSize);

    if (!interpreter) {
        printf("Failed to make interpreter\r\n");
        return;
    }

    auto* input_tensor = interpreter->input_tensor(0);
    unsigned char* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
    if (input_size != static_cast<size_t>(tensorflow::ImageSize(input_dims))) {
        printf("bad input size %d != %d\r\n", input_size, tensorflow::ImageSize(input_dims));
        return;
    }
    tensorflow::ImageDims tensor_dims = {
        input_tensor->dims->data[1],
        input_tensor->dims->data[2],
        input_tensor->dims->data[3]
    };
    if (!tensorflow::ResizeImage({input_dims.height, input_dims.width, input_dims.depth}, input_data.get(), tensor_dims, input_tensor_data)) {
        printf("Failed to resize input image\r\n");
        return;
    }

    if (interpreter->Invoke() != kTfLiteOk) {
        printf("invoke failed\r\n");
        return;
    }

    auto results = tensorflow::GetDetectionResults(interpreter.get(), 0.7, 3);
    for (auto result : results) {
        printf("id: %d score: %f xmin: %f ymin: %f xmax: %f ymax: %f\r\n", result.id, result.score, result.bbox.xmin, result.bbox.ymin, result.bbox.xmax, result.bbox.ymax);
    }
}

}  // namespace valiant

extern "C" void app_main(void *param) {
    valiant::real_main();
    vTaskSuspend(NULL);
}
