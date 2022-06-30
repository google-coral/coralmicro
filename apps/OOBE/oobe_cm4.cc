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

#include "libs/base/ipc_m4.h"
#include "libs/base/led.h"
#include "libs/base/main_freertos_m4.h"
#include "libs/camera/camera.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/detection_responder.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/main_functions.h"

namespace {
bool volatile g_person_detected = false;
bool g_status_led_state = true;

void HandleAppMessage(
    const uint8_t data[coral::micro::ipc::kMessageBufferDataSize],
    void* param) {
  (void)data;
  vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}
}  // namespace

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score) {
  TF_LITE_REPORT_ERROR(error_reporter, "person_score: %d no_person_score: %d",
                       person_score, no_person_score);
// For normal operation, use the person score to determine detection.
// In the OOBE demo, this is handled by a constant timer.
#if !defined(OOBE_DEMO)
  g_person_detected = (person_score > no_person_score);
#endif  // !defined(OOBE_DEMO)
}

extern "C" void app_main(void* param) {
  (void)param;
  coral::micro::IPCM4::GetSingleton()->RegisterAppMessageHandler(
      HandleAppMessage, xTaskGetCurrentTaskHandle());
  coral::micro::CameraTask::GetSingleton()->Init(I2C5Handle());
  coral::micro::CameraTask::GetSingleton()->SetPower(false);
  vTaskDelay(pdMS_TO_TICKS(100));
  coral::micro::CameraTask::GetSingleton()->SetPower(true);
  setup();

  coral::micro::led::Set(coral::micro::led::LED::kStatus, g_status_led_state);
  auto status_led_timer = xTimerCreate(
      "status_led_timer", pdMS_TO_TICKS(1000), pdTRUE, nullptr,
      +[](TimerHandle_t xTimer) {
        g_status_led_state = !g_status_led_state;
        coral::micro::led::Set(coral::micro::led::LED::kStatus,
                               g_status_led_state);
      });
  xTimerStart(status_led_timer, 0);

#if defined(OOBE_DEMO)
  TimerHandle_t m4_timer = xTimerCreate(
      "m4_timer", pdMS_TO_TICKS(10000), pdTRUE, nullptr,
      +[](TimerHandle_t xTimer) { g_person_detected = true; });
  xTimerStart(m4_timer, 0);
#endif  // defined(OOBE_DEMO)

  while (true) {
    printf("M4 main loop\r\n");
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::kStreaming);
    g_person_detected = false;

    while (true) {
      loop();
      coral::micro::led::Set(coral::micro::led::LED::kUser, g_person_detected);

#if !defined(OOBE_SIMPLE)
      if (g_person_detected) {
        break;
      }
#endif
    }
    printf("Person detected, let M7 take over.\r\n");
    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::ipc::Message msg;
    msg.type = coral::micro::ipc::MessageType::kApp;
    coral::micro::IPCM4::GetSingleton()->SendMessage(msg);
    vTaskSuspend(nullptr);
  }
}
