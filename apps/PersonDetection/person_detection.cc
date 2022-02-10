#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/detection_responder.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/main_functions.h"

#include <cstdio>
#include <cstring>
#include <memory>

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score){
    TF_LITE_REPORT_ERROR(error_reporter,
            "person_score: %d no_person_score: %d",
            person_score, no_person_score);

    bool person_detected = person_score > no_person_score;
    valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, person_detected);
    valiant::gpio::SetGpio(valiant::gpio::Gpio::kPowerLED, person_detected);
}

extern "C" void app_main(void *param) {
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);

    setup();
    while (true) {
        loop();
    }
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(NULL);
}
