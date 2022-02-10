#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/peripherals.h"
#include "libs/nxp/rt1176-sdk/pin_mux.h"
#include "libs/tensorflow/hello_world_model.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

#include <cstdio>

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("Stack overflow in %s\r\n", pcTaskName);
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

static void loop() {
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

    ++inference_count;
    if (inference_count >= kInferencesPerCycle) {
        inference_count = 0;
    }
}
static void hello_task(void *param) {
    printf("Starting inference task...\r\n");
    while (true) {
        loop();
        taskYIELD();
    }
}

extern "C" void app_main(void *param) {
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    TF_LITE_REPORT_ERROR(error_reporter, "HelloTensorflowFreeRTOS!");

    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model schema version is %d, supported is %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        vTaskSuspend(NULL);
    }

    static tflite::MicroMutableOpResolver<3> resolver(error_reporter);
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver.AddFullyConnected();
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
        vTaskSuspend(NULL);
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    int ret;
    // High water mark testing showed that this task consumes about 218 words.
    // Set our stack size sufficiently large to accomodate.
    ret = xTaskCreate(hello_task, "HelloTask", configMINIMAL_STACK_SIZE * 3, NULL, configMAX_PRIORITIES - 1, NULL);
    if (ret != pdPASS) {
        printf("Failed to start HelloTask\r\n");
    }
    while (true) {
        taskYIELD();
    }
}
