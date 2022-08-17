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
#include "libs/base/mutex.h"

// Does nothing except start the M4, which runs blink_led_m4
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e multi_core_blink_led
//
// This flashes both the M7 and M4 programs.

extern "C" void app_main(void* param) {
  (void)param;
  printf("Multicore LED Example!\r\n");

  coralmicro::IpcM7::GetSingleton()->StartM4();
  CHECK(coralmicro::IpcM7::GetSingleton()->M4IsAlive(500));
  vTaskSuspend(nullptr);
}
