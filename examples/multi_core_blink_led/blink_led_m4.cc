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

#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Blinks the user LED (green) and status LED (orange) from the M4.
// This is started by main_app_m7.

namespace coralmicro {
namespace {
// [start-sphinx-snippet:blink-led]
[[noreturn]] void Main() {
  printf("M4 started.\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  bool on = true;
  while (true) {
    on = !on;
    LedSet(Led::kUser, on);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
// [end-sphinx-snippet:blink-led]
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void *param) {
  (void)param;
  coralmicro::Main();
}
