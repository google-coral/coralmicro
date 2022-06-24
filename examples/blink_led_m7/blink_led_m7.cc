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
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"

// Blinks the user LED (green), power LED (orange), and Edge TPU LED (white)
// from the M7.

extern "C" [[noreturn]] void app_main(void* param) {
    printf("Blinking LED from M7.\r\n");
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    bool on = true;
    while (true) {
        on = !on;
        coral::micro::led::Set(coral::micro::led::LED::kPower, on);
        coral::micro::led::Set(coral::micro::led::LED::kUser, on);
        coral::micro::led::Set(coral::micro::led::LED::kTpu, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
