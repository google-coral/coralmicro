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
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

// Blinks the user LED (green) and status LED (orange) from the M4.
// This is started by main_app_m7.

// [start-sphinx-snippet:blink-led]
extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M4.\r\n");
    bool on = true;
    while (true) {
        on = !on;
        coralmicro::LedSet(coralmicro::Led::kStatus, on);
        coralmicro::LedSet(coralmicro::Led::kUser, on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
// [end-sphinx-snippet:blink-led]
