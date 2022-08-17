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

#include "libs/base/ipc_m7.h"
#include "libs/base/led.h"
#include "libs/base/mutex.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Runs separate programs on the M7 and M4 core, each
// printing "Hello" messages to the serial console.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e multi_core_hello
//
// This flashes both the M7 and M4 programs.

extern "C" [[noreturn]] void app_main(void *param) {
  (void)param;

  printf("Multicore Hello World Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(coralmicro::Led::kStatus, true);

  coralmicro::IpcM7::GetSingleton()->StartM4();
  CHECK(coralmicro::IpcM7::GetSingleton()->M4IsAlive(500));
  while (true) {
    coralmicro::MulticoreMutexLock lock(0);
    printf("[M7] Hello.\r\n");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
