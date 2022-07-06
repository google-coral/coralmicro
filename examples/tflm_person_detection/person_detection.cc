// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
    coralmicro::led::Set(coralmicro::led::LED::kUser, person_detected);
    coralmicro::led::Set(coralmicro::led::LED::kStatus, person_detected);
}

extern "C" void app_main(void *param) {
    coralmicro::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    coralmicro::CameraTask::GetSingleton()->SetPower(true);
    coralmicro::CameraTask::GetSingleton()->Enable(coralmicro::camera::Mode::kStreaming);

    setup();
    while (true) {
        loop();
    }
    coralmicro::CameraTask::GetSingleton()->Disable();
    coralmicro::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(nullptr);
}
