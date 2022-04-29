#include "libs/testconv1/testconv1.h"

#include "libs/base/filesystem.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coral::micro {
namespace testconv1 {

namespace {
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

const int kModelArenaSize = 32768;
const int kExtraArenaSize = 32768;
const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));
std::vector<uint8_t> testconv1_edgetpu_tflite;
std::vector<uint8_t> testconv1_expected_output_bin;
std::vector<uint8_t> testconv1_test_input_bin;
}  // namespace

bool setup() {
    TfLiteStatus allocate_status;
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    static bool initialized = false;

    if (initialized) {
        return true;
    }

    if (!coral::micro::filesystem::ReadFile("/models/testconv1-edgetpu.tflite", &testconv1_edgetpu_tflite)) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load model!");
        return false;

    }

    if (!coral::micro::filesystem::ReadFile("/models/testconv1-expected-output.bin", &testconv1_expected_output_bin)) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load expected output!");
        return false;
    }

    if (!coral::micro::filesystem::ReadFile("/models/testconv1-test-input.bin", &testconv1_test_input_bin)) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load test input!");
        return false;
    }

    model = tflite::GetModel(testconv1_edgetpu_tflite.data());
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model schema version is %d, supported is %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    static tflite::MicroMutableOpResolver<1> resolver;
    resolver.AddCustom("edgetpu-custom-op", coral::micro::RegisterCustomOp());
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
        return false;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    if (input->bytes != testconv1_test_input_bin.size()) {
        TF_LITE_REPORT_ERROR(error_reporter, "Input tensor length doesn't match canned input");
        return false;
    }
    if (output->bytes != testconv1_expected_output_bin.size()) {
        TF_LITE_REPORT_ERROR(error_reporter, "Output tensor length doesn't match canned output");
        return false;
    }
    memcpy(input->data.uint8, testconv1_test_input_bin.data(), testconv1_test_input_bin.size());

    initialized = true;
    return true;
}

bool loop() {
    memset(output->data.uint8, 0, output->bytes);
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        return false;
    }
    if (memcmp(output->data.uint8, testconv1_expected_output_bin.data(), testconv1_expected_output_bin.size()) != 0) {
        TF_LITE_REPORT_ERROR(error_reporter, "Output did not match expected");
        return false;
    }
    return true;
}

}  // namespace testconv1
}  // namespace coral::micro
