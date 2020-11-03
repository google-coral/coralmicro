#include "apps/HelloTensorflow/board.h"
#include "apps/HelloTensorflow/peripherals.h"
#include "apps/HelloTensorflow/pin_mux.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/tensorflow/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/hello_world/model.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/version.h"

// Run Tensorflow's DebugLog to the debug console.
extern "C" void DebugLog(const char *s) {
    PRINTF(s);
}

namespace {
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

int inference_count = 0;
const int kInferencesPerCycle = 1000;
const float kXrange = 2.f * 3.14159265359f;

const int kModelArenaSize = 4096;
const int kExtraArenaSize = 4096;
const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));
}  // namespace


void loop() {
    float position = static_cast<float>(inference_count) /
                     static_cast<float>(kInferencesPerCycle);
    float x_val = position * kXrange;

    input->data.f[0] = x_val;

    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x_val: %f",
                static_cast<double>(x_val));
        return;
    }

    float y_val = output->data.f[0];

    TF_LITE_REPORT_ERROR(error_reporter, "x_val: %f y_val: %f",
        static_cast<double>(x_val),
        static_cast<double>(y_val));

    // HandleOutput

    ++inference_count;
    if (inference_count >= kInferencesPerCycle) {
        inference_count = 0;
    }
}

int main(int argc, char** argv) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    TF_LITE_REPORT_ERROR(error_reporter, "HelloTensorflow!");

    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model schema version is %d, supported is %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        return -1;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
        return -2;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    while (true) {
        loop();
    }

    return 0;
}
