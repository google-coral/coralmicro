#include "libs/base/filesystem.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace valiant {

namespace {
const int kTensorArenaSize = 1024 * 1024;
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)))
__attribute__((section(".sdram_bss,\"aw\",%nobits @")));
}  // namespace

void real_main() {
    std::vector<uint8_t> model_data;
    if (!filesystem::ReadFile(
            "/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite", &model_data)) {
        printf("Failed to load model\r\n");
        return;
    }

    std::vector<uint8_t> input_data;
    if (!filesystem::ReadFile("/apps/ClassifyImage/cat_224x224.rgb",
                              &input_data)) {
        printf("Failed to load input\r\n");
        return;
    }

    const tflite::Model* model = tflite::GetModel(model_data.data());
    tflite::MicroErrorReporter error_reporter;
    tflite::MicroMutableOpResolver<1> resolver;
    std::shared_ptr<EdgeTpuContext> context =
        EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!context) {
        printf("Failed to get EdgeTpuContext\r\n");
        return;
    }
    std::unique_ptr<tflite::MicroInterpreter> interpreter =
        tensorflow::MakeEdgeTpuInterpreter(model, context.get(), &resolver,
                                           &error_reporter, tensor_arena,
                                           kTensorArenaSize);
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

    if (tensorflow::ClassificationInputNeedsPreprocessing(*input_tensor)) {
        tensorflow::ClassificationPreprocess(input_tensor);
    }
    auto* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
    std::memcpy(input_tensor_data, input_data.data(), input_tensor->bytes);

    if (interpreter->Invoke() != kTfLiteOk) {
        printf("invoke failed\r\n");
        return;
    }

    auto results =
        tensorflow::GetClassificationResults(interpreter.get(), 0.0f, 3);
    for (auto result : results) {
        printf("Label ID: %d Score: %f\r\n", result.id, result.score);
    }
}

}  // namespace valiant

extern "C" void app_main(void* param) {
    valiant::real_main();
    vTaskSuspend(NULL);
}
