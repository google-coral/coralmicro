#include <cstring>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/tensorflow/classification.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace valiant {
namespace {
constexpr char kModelPath[] =
    "/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite";
constexpr char kImagePath[] = "/apps/ClassifyImage/cat_224x224.rgb";
constexpr int kTensorArenaSize = 1024 * 1024;
struct TensorArena {
    alignas(16) uint8_t data[kTensorArenaSize];
};

void Main() {
    std::vector<uint8_t> model;
    if (!filesystem::ReadFile(kModelPath, &model)) {
        printf("ERROR: Failed to load %s\r\n", kModelPath);
        return;
    }

    std::vector<uint8_t> image;
    if (!filesystem::ReadFile(kImagePath, &image)) {
        printf("ERROR: Failed to load %s\r\n", kImagePath);
        return;
    }

    auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!tpu_context) {
        printf("ERROR: Failed to get EdgeTpu context\r\n");
        return;
    }

    tflite::MicroErrorReporter error_reporter;
    tflite::MicroMutableOpResolver<1> resolver;
    resolver.AddCustom(kCustomOp, RegisterCustomOp());

    // As an alternative check STATIC_TENSOR_ARENA_IN_SDRAM macro.
    auto tensor_arena = std::make_unique<TensorArena>();
    tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()),
                                         resolver, tensor_arena->data,
                                         kTensorArenaSize, &error_reporter);
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        printf("ERROR: AllocateTensors() failed\r\n");
        return;
    }

    if (interpreter.inputs().size() != 1) {
        printf("ERROR: Model must have only one input tensor\r\n");
        return;
    }

    auto* input_tensor = interpreter.input_tensor(0);
    if (input_tensor->type != kTfLiteUInt8 ||
        input_tensor->bytes != image.size()) {
        printf("ERROR: Invalid input tensor size\r\n");
        return;
    }

    std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image.data(),
                image.size());

    if (interpreter.Invoke() != kTfLiteOk) {
        printf("ERROR: Invoke() failed\r\n");
        return;
    }

    auto results = tensorflow::GetClassificationResults(&interpreter, 0.0f, 3);
    for (auto& result : results)
        printf("Label ID: %d Score: %f\r\n", result.id, result.score);
}
}  // namespace
}  // namespace valiant

extern "C" void app_main(void* param) {
    (void)param;
    valiant::Main();
    vTaskSuspend(nullptr);
}
