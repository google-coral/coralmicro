#include "libs/base/filesystem.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/detection_responder.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/main_functions.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/person_detect_model_data.h"

#include <cstdio>
#include <cstring>
#include <memory>

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score){
    TF_LITE_REPORT_ERROR(error_reporter,
            "person_score: %d no_person_score: %d",
            person_score, no_person_score);

    GPIO_PinWrite(GPIO13, 5, person_score > no_person_score);
    GPIO_PinWrite(GPIO13, 6, person_score > no_person_score);
}

extern "C" void app_main(void *param) {
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable();
    gpio_pin_config_t user_led = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO13, 6, &user_led);
    gpio_pin_config_t pwr_led = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO13, 5, &pwr_led);

    setup();
    while (true) {
        loop();
    }
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(NULL);
}
