#include <cstring>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/tensorflow/detection.h"
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
    "/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite";
constexpr char kImagePath[] = "/apps/DetectImage/cat_300x300.rgb";
constexpr int kTensorArenaSize = 8 * 1024 * 1024;
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
    tflite::MicroMutableOpResolver<3> resolver;
    resolver.AddDequantize();
    resolver.AddDetectionPostprocess();
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

    auto results = tensorflow::GetDetectionResults(&interpreter, 0.6, 3);
    for (auto result : results) {
        printf("id: %d score: %f xmin: %f ymin: %f xmax: %f ymax: %f\r\n",
               result.id, result.score, result.bbox.xmin, result.bbox.ymin,
               result.bbox.xmax, result.bbox.ymax);
    }
}
}  // namespace
}  // namespace valiant

extern "C" void app_main(void* param) {
    valiant::Main();
    vTaskSuspend(nullptr);
}
