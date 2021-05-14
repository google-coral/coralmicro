#include "third_party/freertos_kernel/include/projdefs.h"
#include "libs/base/filesystem.h"
#include "libs/base/tasks_m7.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tensorflow/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/version.h"

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

size_t testconv1_edgetpu_tflite_len, testconv1_test_input_bin_len, testconv1_expected_output_bin_len;
const uint8_t *testconv1_edgetpu_tflite;
const uint8_t *testconv1_expected_output_bin;
const uint8_t *testconv1_test_input_bin;
}  // namespace

bool loop() {
    memset(output->data.uint8, 0, output->bytes);
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        return false;
    }
    if (memcmp(output->data.uint8, testconv1_expected_output_bin, testconv1_expected_output_bin_len) != 0) {
        TF_LITE_REPORT_ERROR(error_reporter, "Output did not match expected");
        return false;
    }
    return true;
}

static bool setup() {
    TfLiteStatus allocate_status;
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    TF_LITE_REPORT_ERROR(error_reporter, "TestConv1!");

    testconv1_edgetpu_tflite = valiant::filesystem::ReadToMemory("/models/testconv1-edgetpu.tflite", &testconv1_edgetpu_tflite_len);
    testconv1_expected_output_bin = valiant::filesystem::ReadToMemory("/models/testconv1-expected-output.bin", &testconv1_expected_output_bin_len);
    testconv1_test_input_bin = valiant::filesystem::ReadToMemory("/models/testconv1-test-input.bin", &testconv1_test_input_bin_len);

    if (!testconv1_edgetpu_tflite || testconv1_edgetpu_tflite_len == 0) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load model!");
        return false;
    }
    if (!testconv1_expected_output_bin || testconv1_expected_output_bin_len == 0) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load expected output!");
        return false;
    }
    if (!testconv1_test_input_bin || testconv1_test_input_bin_len == 0) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load test input!");
        return false;
    }

    model = tflite::GetModel(testconv1_edgetpu_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model schema version is %d, supported is %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    static tflite::MicroMutableOpResolver<1> resolver;
    resolver.AddCustom("edgetpu-custom-op", valiant::RegisterCustomOp());
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

    if (input->bytes != testconv1_test_input_bin_len) {
        TF_LITE_REPORT_ERROR(error_reporter, "Input tensor length doesn't match canned input");
        return false;
    }
    if (output->bytes != testconv1_expected_output_bin_len) {
        TF_LITE_REPORT_ERROR(error_reporter, "Output tensor length doesn't match canned output");
        return false;
    }
    memcpy(input->data.uint8, testconv1_test_input_bin, testconv1_test_input_bin_len);
    return true;
}

extern "C" void app_main(void *param) {
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();

    if (!setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(NULL);
    }

    bool run = true;
    size_t counter = 0;
    while (true) {
        if (run) {
            run = loop();
            ++counter;
            if ((counter % 100) == 0) {
                printf("Execution %u...\r\n", counter);
            }
        }
    }
    vTaskSuspend(NULL);
}
