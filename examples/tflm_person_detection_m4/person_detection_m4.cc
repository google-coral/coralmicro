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

#include "libs/base/led.h"
#include "libs/base/main_freertos_m4.h"
#include "libs/base/timer.h"
#include "libs/camera/camera.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/detection_responder.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/main_functions.h"

void RespondToDetection(tflite::ErrorReporter*, int8_t person_score,
                        int8_t no_person_score) {
  printf("person_score: %d no_person_score: %d\r\n", person_score,
         no_person_score);

  bool person_detected = person_score > no_person_score;
  LedSet(coralmicro::Led::kUser, person_detected);
}

namespace coralmicro {
[[noreturn]] void Main() {
  printf("M4 awake!\r\n");
  CameraTask::GetSingleton()->Init(I2C5Handle());
  CameraTask::GetSingleton()->SetPower(false);
  vTaskDelay(pdMS_TO_TICKS(100));
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);

  setup();
  while (true) {
    auto start = TimerMicros();
    loop();
    printf("Latency: %lu us\r\n", static_cast<uint32_t>(TimerMicros() - start));
  }
}
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
