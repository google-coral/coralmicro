#include "libs/base/filesystem.h"
#include "libs/base/led.h"
#include "libs/camera/camera.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/detection_responder.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/main_functions.h"

#include <cstdio>
#include <cstring>
#include <memory>

// Runs a 250 kB TFLM model that recognizes people with the camera on the
// Dev Board Micro, printing scores for "person" and "no person" in the serial
// console. The model runs on the M7 core alone; it does NOT use the Edge TPU.
//
// For more information about this model, see:
// https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/person_detection

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score){
    TF_LITE_REPORT_ERROR(error_reporter,
            "person_score: %d no_person_score: %d",
            person_score, no_person_score);

    bool person_detected = person_score > no_person_score;
    coral::micro::led::Set(coral::micro::led::LED::kUser, person_detected);
    coral::micro::led::Set(coral::micro::led::LED::kPower, person_detected);
}

extern "C" void app_main(void *param) {
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(coral::micro::camera::Mode::STREAMING);

    setup();
    while (true) {
        loop();
    }
    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(nullptr);
}
