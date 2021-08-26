#include "libs/tensorflow/utils.h"


namespace valiant {
namespace tensorflow {

std::unique_ptr<tflite::MicroInterpreter> MakeEdgeTpuInterpreterInternal(
    const tflite::Model *model,
    EdgeTpuContext *context,
    tflite::MicroOpResolver* resolver,
    tflite::MicroErrorReporter* error_reporter,
    uint8_t *tensor_arena,
    size_t tensor_arena_size)
{
    if (!model || !error_reporter) {
        return nullptr;
    }

    std::unique_ptr<tflite::MicroInterpreter> interpreter(new tflite::MicroInterpreter(model, *resolver, tensor_arena, tensor_arena_size, error_reporter));
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
        return nullptr;
    }

    return interpreter;
}

}  // namespace tensorflow
}  // namespace valiant
