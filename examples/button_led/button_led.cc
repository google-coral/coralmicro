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
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Toggles the User LED in response to button presses.
extern "C" [[noreturn]] void app_main(void* param) {
    printf("Press the user button.\r\n");
    auto app_main_handle = xTaskGetCurrentTaskHandle();
    CHECK(app_main_handle);

    // Register callback for the user button.
    coralmicro::GpioRegisterIrqHandler(
        coralmicro::Gpio::kUserButton,
        [&app_main_handle]() { xTaskResumeFromISR(app_main_handle); });
    bool user_led_on{false};
    while (true) {
        vTaskSuspend(nullptr);
        user_led_on = !user_led_on;
        coralmicro::LedSet(coralmicro::Led::kUser, user_led_on);
    }
}
//! [gpio-callback] End snippet
