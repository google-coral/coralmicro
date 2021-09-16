#include "libs/base/filesystem.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace valiant {

namespace {
    const int kTensorArenaSize = 1024 * 1024;
    static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
}  // namespace

void real_main() {
    size_t model_size, input_size;
    std::unique_ptr<uint8_t> model_data(filesystem::ReadToMemory("/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite", &model_size));
    std::unique_ptr<uint8_t> input_data(filesystem::ReadToMemory("/apps/ClassifyImage/cat.rgb", &input_size));
    if (!model_data || model_size == 0) {
        printf("Failed to load model\r\n");
        return;
    }

    if (!input_data || input_size == 0) {
        printf("Failed to load input\r\n");
        return;
    }
    const tflite::Model* model = tflite::GetModel(model_data.get());
    std::unique_ptr<tflite::MicroErrorReporter> error_reporter(new tflite::MicroErrorReporter());
    std::unique_ptr<tflite::MicroMutableOpResolver<1>> resolver(new tflite::MicroMutableOpResolver<1>);
    std::shared_ptr<EdgeTpuContext> context = EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!context) {
        printf("Failed to get EdgeTpuContext\r\n");
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
    if (interpreter->inputs().size() != 1) {
        printf("Bad inputs size\r\n");
        return;
    }
    auto* input_tensor = interpreter->input_tensor(0);
    if (input_tensor->type != kTfLiteUInt8) {
        printf("Bad input type\r\n");
        return;
    }

    unsigned char* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
    if (tensorflow::ClassificationInputNeedsPreprocessing(*input_tensor)) {
        printf("must preprocess\r\n");
        return;
    } else {
        memcpy(input_tensor_data, input_data.get(), input_tensor->bytes);
    }

    if (interpreter->Invoke() != kTfLiteOk) {
        printf("invoke failed\r\n");
        return;
    }

    auto results = tensorflow::GetClassificationResults(interpreter.get(), 0.0f, 3);
    for (auto result : results) {
        printf("Label ID: %d Score: %f\r\n", result.id, result.score);
    }
}

}  // namespace valiant

extern "C" void app_main(void *param) {
    valiant::real_main();
    vTaskSuspend(NULL);
}
