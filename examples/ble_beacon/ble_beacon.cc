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
#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Broadcasts device details to allow discovery with Bluetooth.
//
// Requires the Coral Wireless Add-on.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e ble_beacon

extern "C" void app_main(void* param) {
  (void)param;
  printf("BLE Beacon Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(coralmicro::Led::kStatus, true);

  InitEdgefastBluetooth(nullptr);

  while (!BluetoothReady()) {
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  printf("Advertising bluetooth...\r\n");
  if (auto advertised = BluetoothAdvertise(); !advertised) {
    printf("Failed to advertise bluetooth...\r\n");
  }

  vTaskSuspend(nullptr);
}
