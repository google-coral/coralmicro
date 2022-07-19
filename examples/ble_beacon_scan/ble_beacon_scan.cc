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
// https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/bluetooth/scan_adv/src/main.c

#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace {
void scan_cb(const bt_addr_le_t* addr, int8_t rssi, uint8_t adv_type,
             struct net_buf_simple* buf) {
  static char addr_s[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(addr, addr_s, sizeof(addr_s));

  printf("Scan: find device %s\r\n", addr_s);
}

void bt_ready(int err) {
  struct bt_le_scan_param scan_param = {
      .type = BT_HCI_LE_SCAN_ACTIVE,
      .options = BT_LE_SCAN_OPT_NONE,
      .interval = 0x0800,
      .window = 0x0010,
  };

  err = bt_le_scan_start(&scan_param, scan_cb);
  if (err) {
    printf("Starting scanning failed (err %d)\n", err);
    return;
  }
}
}  // namespace

extern "C" void app_main(void* param) {
  (void)param;
  InitEdgefastBluetooth(bt_ready);
  vTaskSuspend(nullptr);
}
