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

#include <cstdio>

#include "libs/base/check.h"
#include "libs/base/led.h"
#include "libs/camera/camera.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"

// Uses the camera hardware's motion interrupt to turn on the User LED
// when motion is detected.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e camera_motion_detection

// [start-snippet:motion-detection]
namespace coralmicro {
namespace {
void Main() {
  printf("Camera Motion Detection example\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  TimerHandle_t motion_detection_timer = xTimerCreate(
      "motion_detection_timer", pdMS_TO_TICKS(200), pdFALSE, nullptr,
      [](TimerHandle_t timer) { LedSet(Led::kUser, false); });
  CHECK(motion_detection_timer);

  // Enable Power, configure motion detection, and enable streaming.
  CameraTask::GetSingleton()->SetPower(true);

  CameraMotionDetectionConfig config{};
  CameraTask::GetSingleton()->GetMotionDetectionConfigDefault(config);
  config.cb = [](void* param) {
    auto timer = reinterpret_cast<TimerHandle_t>(param);
    CHECK(timer);
    printf("Motion detected\r\n");
    LedSet(Led::kUser, true);
    if (xTimerIsTimerActive(timer) == pdTRUE) {
      xTimerReset(timer, portMAX_DELAY);
    } else {
      xTimerStart(timer, portMAX_DELAY);
    }
  };
  config.cb_param = motion_detection_timer;

  CameraTask::GetSingleton()->SetMotionDetectionConfig(config);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);

  vTaskSuspend(nullptr);
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
// [end-snippet:motion-detection]
