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

#include "apps/mfg_test/mfg_test_iperf.h"
#include "libs/base/check.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/mutex.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"
#include "libs/base/wifi.h"
#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"

extern "C" {
#include "third_party/modified/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

namespace {
using coralmicro::JsonRpcGetStringParam;

void WiFiGetAP(struct jsonrpc_request* request) {
  std::string name;
  if (!JsonRpcGetStringParam(request, "name", &name)) return;

  WIFIReturnCode_t wifi_ret;
  constexpr int kNumResults = 50;
  WIFIScanResult_t xScanResults[kNumResults] = {0};
  wifi_ret = WIFI_Scan(xScanResults, kNumResults);

  if (wifi_ret != eWiFiSuccess) {
    jsonrpc_return_error(request, -1, "wifi scan failed", nullptr);
    return;
  }

  std::vector<int> scan_indices;
  for (int i = 0; i < kNumResults; ++i) {
    if (memcmp(xScanResults[i].cSSID, name.c_str(), name.size()) == 0) {
      scan_indices.push_back(i);
    }
  }

  if (scan_indices.empty()) {
    jsonrpc_return_error(request, -1, "network not found", nullptr);
    return;
  }

  int8_t best_rssi = SCHAR_MIN;
  for (auto scan_indice : scan_indices) {
    if (xScanResults[scan_indice].cRSSI > best_rssi) {
      best_rssi = xScanResults[scan_indice].cRSSI;
    }
  }

  jsonrpc_return_success(request, "{%Q:%d}", "signal_strength", best_rssi);
}

SemaphoreHandle_t ble_ready_mtx;
bool ble_ready = false;
SemaphoreHandle_t ble_scan_sema;

struct bt_le_scan_param scan_param = {
  .type = BT_HCI_LE_SCAN_ACTIVE,
  .options = BT_LE_SCAN_OPT_NONE,
  .interval = 0x800,
  .window = 0x10,
};

void BLEFind(struct jsonrpc_request* request) {
  {
    coralmicro::MutexLock lock(ble_ready_mtx);
    if (!ble_ready) {
      jsonrpc_return_error(request, -1, "bt not ready yet", nullptr);
      return;
    }
  }

  std::string address;
  if (!JsonRpcGetStringParam(request, "address", &address)) return;

  static bt_addr_t bt_addr;
  if (bt_addr_from_str(address.c_str(), &bt_addr)) {
    jsonrpc_return_error(request, -1, "could not get six octets from 'address'",
                         nullptr);
    return;
  }

  static int8_t rssi;
  // This static here is because there isn't a way to pass a parameter into
  // the scan. Reset the value each time through the method.
  static bool found_match;
  found_match = false;

  int err = bt_le_scan_start(&scan_param, [](const bt_addr_le_t* addr, int8_t ret_rssi, uint8_t adv_type, struct net_buf_simple* buf) {
    rssi = ret_rssi;
    if (memcmp(&bt_addr, &addr->a, sizeof(bt_addr)) == 0) {
      bt_le_scan_stop();
      found_match = true;
      CHECK(xSemaphoreGive(ble_scan_sema) == pdTRUE);
    }
  });
  if (err) {
    jsonrpc_return_error(request, -1, "failed to initiate bt scan", nullptr);
    return;
  }
  if (xSemaphoreTake(ble_scan_sema, pdMS_TO_TICKS(10000)) != pdTRUE) {
    found_match = false;
    bt_le_scan_stop();
  }
  if (found_match) {
    jsonrpc_return_success(request, "{%Q:%d}", "signal_strength", rssi);
  } else {
    jsonrpc_return_error(request, -1, "failed to find 'address'", nullptr);
  }
}

void BLEScan(struct jsonrpc_request* request) {
  {
    coralmicro::MutexLock lock(ble_ready_mtx);
    if (!ble_ready) {
      jsonrpc_return_error(request, -1, "bt not ready yet", nullptr);
      return;
    }
  }

  static int8_t rssi;
  static char address[18] = "00:00:00:00:00:00";


  // This static here is because there isn't a way to pass a parameter into
  // the scan. Reset the value each time through the method.
  static bool found_address;
  found_address = false;
  int err = bt_le_scan_start(&scan_param, [](const bt_addr_le_t* addr, int8_t ret_rssi, uint8_t adv_type, struct net_buf_simple* buf) {
    found_address = true;
    bt_addr_le_to_str(addr, address, sizeof(address));
    rssi = ret_rssi;
    bt_le_scan_stop();
    CHECK(xSemaphoreGive(ble_scan_sema) == pdTRUE);
  });
  if (err) {
    jsonrpc_return_error(request, -1, "failed to initiate bt scan", nullptr);
    return;
  }
  CHECK(xSemaphoreTake(ble_scan_sema, portMAX_DELAY) == pdTRUE);
  if (found_address) {
    jsonrpc_return_success(request, "{%Q:%Q, %Q:%d}", "address", address,
                           "signal_strength", rssi);
  } else {
    jsonrpc_return_error(request, -1, "failed to find 'address'", nullptr);
  }
}

void bt_ready(int err) {
  coralmicro::MutexLock lock(ble_ready_mtx);
  if (!err) {
    ble_ready = true;
  }
}

}  // namespace

extern "C" void app_main(void* param) {
  ble_ready_mtx = xSemaphoreCreateMutex();
  CHECK(ble_ready_mtx);
  ble_scan_sema = xSemaphoreCreateBinary();
  CHECK(ble_scan_sema);

  if (coralmicro::WiFiTurnOn(/*default_iface=*/false)) {
    if (coralmicro::WiFiConnect()) {
      coralmicro::LedSet(coralmicro::Led::kUser, true);
    }
  } else {
    printf("Wi-Fi failed to come up (is the Wi-Fi board attached?\r\n");
    coralmicro::LedSet(coralmicro::Led::kStatus, true);
    vTaskSuspend(nullptr);
  }
  InitEdgefastBluetooth(bt_ready);

  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiScan,
                 coralmicro::testlib::WiFiScan);
  jsonrpc_export("wifi_get_ap", WiFiGetAP);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiSetAntenna,
                 coralmicro::testlib::WiFiSetAntenna);
  jsonrpc_export("ble_scan", BLEScan);
  jsonrpc_export("ble_find", BLEFind);
  IperfInit();
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
  vTaskSuspend(nullptr);
}
