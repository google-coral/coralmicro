#include "third_party/freertos_kernel/include/projdefs.h"
#include "libs/base/tasks_m7.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "libs/tensorflow/testconv1_edgetpu.h"
#include "libs/tensorflow/testconv1_expected_output.h"
#include "libs/tensorflow/testconv1_test_input.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/tensorflow/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/version.h"

// Run Tensorflow's DebugLog to the debug console.
extern "C" void DebugLog(const char *s) {
    printf(s);
}

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
}  // namespace

// Measured stack usage: 242 words
void UsbHostTask(void *param) {
    while (true) {
        valiant::UsbHostTask::GetSingleton()->UsbHostTaskFn();
        taskYIELD();
    }
}

// Measured stack usage: 146 words
void DfuTask(void *param) {
    while (true) {
        valiant::EdgeTpuDfuTask::GetSingleton()->EdgeTpuDfuTaskFn();
        taskYIELD();
    }
}

void EdgeTpuTask(void *param) {
    while (true) {
        valiant::EdgeTpuTask::GetSingleton()->EdgeTpuTaskFn();
        taskYIELD();
    }
}

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
        printf("Input tensor length doesn't match canned input\r\n");
        return false;
    }
    if (output->bytes != testconv1_expected_output_bin_len) {
        printf("Output tensor length doesn't match canned output\r\n");
        return false;
    }
    memcpy(input->data.uint8, testconv1_test_input_bin, testconv1_test_input_bin_len);
    return true;
}

extern "C" void app_main(void *param) {
    int ret;

    ret = xTaskCreate(UsbHostTask, "UsbHostTask", configMINIMAL_STACK_SIZE * 10, NULL, APP_TASK_PRIORITY + 1, NULL);
    if (ret != pdPASS) {
        printf("Failed to start UsbHostTask\r\n");
        return;
    }

    ret = xTaskCreate(DfuTask, "DfuTask", configMINIMAL_STACK_SIZE * 3, NULL, APP_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        printf("Failed to start DfuTask\r\n");
        return;
    }

    ret = xTaskCreate(EdgeTpuTask, "EdgeTpuTask", configMINIMAL_STACK_SIZE * 10, NULL, APP_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        printf("Failed to start EdgeTpuTask\r\n");
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Can't interrupt on 8/23
    gpio_pin_config_t boot_fail_config = {kGPIO_DigitalInput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO8, 23, &boot_fail_config);

    // Can't interrupt on 8/26
    gpio_pin_config_t pgood_config = {kGPIO_DigitalInput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO8, 26, &pgood_config);

    gpio_pin_config_t rst_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO8, 24, &rst_config);
    GPIO_PinWrite(GPIO8, 24, 0);

    gpio_pin_config_t pmic_config = {kGPIO_DigitalOutput, 1, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO8, 25, &pmic_config);
    GPIO_PinWrite(GPIO8, 25, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    GPIO_PinWrite(GPIO8, 25, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    bool pgood;
    do {
        pgood = !!GPIO_PinRead(GPIO8, 26);
    } while (!pgood);
    printf("PGOOD is up\r\n");

    vTaskDelay(pdMS_TO_TICKS(1000));
    GPIO_PinWrite(GPIO8, 24, 1);
    printf("Released reset\r\n");

    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();
    setup();

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
}
