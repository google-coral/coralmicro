#include "libs/base/filesystem.h"
#include "libs/tensorflow/detection.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/schema/schema_generated.h"

#include <cstdio>

namespace valiant {

namespace {
    constexpr int kTensorArenaSize = 8 * 1024 * 1024;
    static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
}  // namespace

void real_main() {
    size_t model_size, input_size;
    auto model_data = filesystem::ReadToMemory("/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite", &model_size);
    if (!model_data || model_size == 0) {
        printf("Failed to load model %p %d\r\n", model_data.get(), model_size);
        return;
    }

    auto input_data = filesystem::ReadToMemory("/apps/DetectImage/cat_300x300.rgb", &input_size);
    if (!input_data || input_size == 0) {
        printf("Failed to load input\r\n");
        return;
    }

    const tflite::Model* model = tflite::GetModel(model_data.get());

    constexpr int kNumOps = 3;
    tflite::MicroMutableOpResolver<kNumOps> resolver;
    resolver.AddDequantize();
    resolver.AddDetectionPostprocess();

    std::shared_ptr<EdgeTpuContext> context = EdgeTpuManager::GetSingleton()->OpenDevice(PerformanceMode::kMax);
    if (!context) {
        printf("Failed to get TPU context\r\n");
        return;
    }

    tflite::MicroErrorReporter error_reporter;
    std::unique_ptr<tflite::MicroInterpreter> interpreter =
        tensorflow::MakeEdgeTpuInterpreter(model, context.get(),
        &resolver, &error_reporter,
        tensor_arena, kTensorArenaSize);
    if (!interpreter) {
        printf("Failed to make interpreter\r\n");
        return;
    }

    auto* input_tensor = interpreter->input_tensor(0);
    auto* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
    if (input_size != input_tensor->bytes) {
        printf("bad input size %d != %d\r\n", input_size, input_tensor->bytes);
        return;
    }

    std::memcpy(input_tensor_data, input_data.get(), input_tensor->bytes);

    if (interpreter->Invoke() != kTfLiteOk) {
        printf("invoke failed\r\n");
        return;
    }

    auto results = tensorflow::GetDetectionResults(interpreter.get(), 0.6, 3);
    for (auto result : results) {
        printf("id: %d score: %f xmin: %f ymin: %f xmax: %f ymax: %f\r\n", result.id, result.score, result.bbox.xmin, result.bbox.ymin, result.bbox.xmax, result.bbox.ymax);
    }
}

}  // namespace valiant

extern "C" void app_main(void *param) {
    valiant::real_main();
    vTaskSuspend(NULL);
}
