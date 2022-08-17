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

// Port of Zephyr's beacon sample.
// Original sample at:
// https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/bluetooth/beacon/src/main.c

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

#define DEVICE_NAME "Coral Dev Board Micro"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

namespace {
// Set Advertisement data. Based on the Eddystone specification:
// https://github.com/google/eddystone/blob/master/protocol-specification.md
// https://github.com/google/eddystone/tree/master/eddystone-url
const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
    BT_DATA_BYTES(BT_DATA_SVC_DATA16, 0xaa, 0xfe, /* Eddystone UUID */
                  0x10,                           /* Eddystone-URL frame type */
                  0x00, /* Calibrated Tx power at 0m */
                  0x03, /* URL Scheme Prefix https:// */
                  'c', 'o', 'r', 'a', 'l', '.', 'a', 'i')};

// Set Scan Response data
const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

void bt_ready(int err) {
  char addr_s[BT_ADDR_LE_STR_LEN];
  bt_addr_le_t addr = {0};
  size_t count = 1;

  if (err) {
    printf("bt init failed: %d\r\n", err);
    return;
  }

  // https://github.com/zephyrproject-rtos/zephyr/pull/24829#issuecomment-622977341
  const auto& params = BT_LE_ADV_NCONN_IDENTITY;
  err = bt_le_adv_start(params, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
  if (err) {
    printf("failed to start advertising\r\n");
    return;
  }

  bt_id_get(&addr, &count);
  bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

  printf("Beacon started, advertising as %s\r\n", addr_s);
}

}  // namespace

extern "C" void app_main(void* param) {
  (void)param;
  printf("BLE Beacon Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(coralmicro::Led::kStatus, true);

  InitEdgefastBluetooth(bt_ready);
  vTaskSuspend(nullptr);
}
