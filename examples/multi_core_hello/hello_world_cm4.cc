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

#include "libs/base/mutex.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// This program runs on the M4. It must be started by the M7 program.

extern "C" [[noreturn]] void app_main(void *param) {
  (void)param;
  printf("M4 started.\r\n");

  while (true) {
    coralmicro::MulticoreMutexLock lock(0);
    printf("[M4] Hello.\r\n");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
