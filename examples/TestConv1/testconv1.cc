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

#include "libs/testconv1/testconv1.h"

#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/tpu/edgetpu_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/projdefs.h"

extern "C" [[noreturn]] void app_main(void* param) {
    if (!coral::micro::testconv1::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }

    size_t counter = 0;
    while (true) {
        auto tpu_context =
            coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();
        coral::micro::led::Set(coral::micro::led::LED::kTpu, true);

        bool run = true;
        for (int i = 0; i < 1000; ++i) {
            if (run) {
                run = coral::micro::testconv1::loop();
                ++counter;
                if ((counter % 100) == 0) {
                    printf("Execution %u...\r\n", counter);
                }
            }
        }

        printf("Reset EdgeTPU...\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskSuspend(nullptr);
}
