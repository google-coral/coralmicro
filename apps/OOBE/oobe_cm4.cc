#include "libs/base/ipc.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/detection_responder.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/main_functions.h"
#include <cstdio>

static bool g_person_detected = false;

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score){
    TF_LITE_REPORT_ERROR(error_reporter,
            "person_score: %d no_person_score: %d",
            person_score, no_person_score);

    GPIO_PinWrite(GPIO13, 6, person_score > no_person_score);

    g_person_detected = (person_score > no_person_score);
}

static void HandleAppMessage(const uint8_t data[valiant::ipc::kMessageBufferDataSize], void *param) {
    vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}

extern "C" void app_main(void *param) {
    valiant::IPC::GetSingleton()->RegisterAppMessageHandler(HandleAppMessage, xTaskGetCurrentTaskHandle());
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    valiant::CameraTask::GetSingleton()->SetPower(true);
    setup();
    GPIO_PinWrite(GPIO13, 6, 1);
    while (true) {
        printf("M4 main loop\r\n");
        valiant::CameraTask::GetSingleton()->Enable();
        gpio_pin_config_t user_led = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
        GPIO_PinInit(GPIO13, 6, &user_led);

        while (true) {
            loop();
            if (g_person_detected) {
                break;
            }
        }
        printf("Person detected, let M7 take over.\r\n");
        valiant::CameraTask::GetSingleton()->Disable();
        valiant::ipc::Message msg;
        msg.type = valiant::ipc::MessageType::APP;
        valiant::IPC::GetSingleton()->SendMessage(msg);
        vTaskSuspend(NULL);
    }
}
