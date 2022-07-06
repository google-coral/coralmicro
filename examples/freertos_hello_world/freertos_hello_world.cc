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

#include "libs/base/console_m7.h"
#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/pmic/pmic.h"
#include "libs/tpu/edgetpu_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

[[noreturn]] void read_task(void* param) {
    char ch;
    do {
        int bytes = coralmicro::ConsoleM7::GetSingleton()->Read(&ch, 1);
        if (bytes == 1) {
            coralmicro::ConsoleM7::GetSingleton()->Write(&ch, 1);
        }
        taskYIELD();
    } while (true);
}

extern "C" [[noreturn]] void app_main(void* param) {
    printf("Hello world FreeRTOS.\r\n");

    coralmicro::PmicTask::GetSingleton()->SetRailState(
        coralmicro::pmic::Rail::kCam2V8, true);
    coralmicro::PmicTask::GetSingleton()->SetRailState(
        coralmicro::pmic::Rail::kCam1V8, true);
    coralmicro::PmicTask::GetSingleton()->SetRailState(
        coralmicro::pmic::Rail::kMic1V8, true);
    coralmicro::EdgeTpuTask::GetSingleton()->SetPower(true);

    xTaskCreate(read_task, "read_task", configMINIMAL_STACK_SIZE, nullptr,
                APP_TASK_PRIORITY, nullptr);

    bool up = true;
    unsigned int brightness = 50;
    while (true) {
        coralmicro::led::Set(coralmicro::led::LED::kStatus, brightness > 50);
        coralmicro::led::Set(coralmicro::led::LED::kUser, brightness > 50);
        coralmicro::led::Set(coralmicro::led::LED::kTpu, true, brightness);

        if (up) {
            ++brightness;
        } else {
            --brightness;
        }
        if (brightness == 100) {
            up = false;
        }
        if (brightness == 0) {
            up = true;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskSuspend(nullptr);
}
