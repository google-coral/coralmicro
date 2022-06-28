/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/mutex.h"

/* clang-format off */
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include/wiced_management.h"
#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"
#include "third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller.h"
#include "third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/include/BT_hci_api.h"
#include "third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.h"
/* clang-format on */

namespace {
constexpr char kDeviceName[] = "Coral Dev Board Micro";
constexpr size_t kDeviceNameLen = 21;

// Set Advertisement data. Based on the Eddystone specification:
// https://github.com/google/eddystone/blob/master/protocol-specification.md
// https://github.com/google/eddystone/tree/master/eddystone-url
const struct bt_data kAdvertiseData[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
    BT_DATA_BYTES(BT_DATA_SVC_DATA16, 0xaa, 0xfe, /* Eddystone UUID */
                  0x10,                           /* Eddystone-URL frame type */
                  0x00, /* Calibrated Tx power at 0m */
                  0x03, /* URL Scheme Prefix https:// */
                  'c', 'o', 'r', 'a', 'l', '.', 'a', 'i')};

// Set Scan Response data.
const struct bt_data kScanResponseData[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, kDeviceName, kDeviceNameLen),
};

bt_ready_cb_t g_init_cb = nullptr;

SemaphoreHandle_t g_ble_scan_mtx;
bool g_bt_initialized = false;            // Protected by g_ble_scan_mtx.
int g_max_num_results;                    // Protected by g_ble_scan_mtx.
std::vector<std::string> g_scan_results;  // Protected by g_ble_scan_mtx.
}  // namespace

extern unsigned char brcm_patchram_buf[];
extern unsigned int brcm_patch_ram_length;
void ble_pwr_on(void);

extern "C" lfs_t* lfs_pl_init() { return coralmicro::Lfs(); }

extern "C" int controller_hci_uart_get_configuration(
    controller_hci_uart_config_t* config) {
  if (!config) {
    return -1;
  }
  config->clockSrc = CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart2);
  config->defaultBaudrate = 115200;
  config->runningBaudrate = 115200;
  config->instance = 2;
  config->enableRxRTS = 1;
  config->enableTxCTS = 1;
  return 0;
}

static void bt_ready_internal(int err_param) {
  if (err_param) {
    printf("Bluetooth initialization failed: %d\r\n", err_param);
    return;
  }

  // Kick the Bluetooth module into patchram download mode.
  constexpr int kCmdDownloadMode = 0x2E;
  int err = bt_hci_cmd_send_sync(BT_OP(BT_OGF_VS, kCmdDownloadMode), nullptr,
                                 nullptr);
  if (err) {
    printf("Initializing patchram download failed: %d\r\n", err);
    return;
  }
  // Sleep to allow the transition into download mode.
  vTaskDelay(pdMS_TO_TICKS(50));

  // The patchram file consists of raw HCI commands.
  // Build command buffers and send them to the module.
  size_t offset = 0;
  while (offset != brcm_patch_ram_length) {
    uint16_t opcode = *(uint16_t*)&brcm_patchram_buf[offset];
    uint8_t len = brcm_patchram_buf[offset + sizeof(uint16_t)];
    offset += sizeof(uint16_t) + sizeof(uint8_t);
    struct net_buf* buf = bt_hci_cmd_create(opcode, len);
    auto* dat = reinterpret_cast<uint8_t*>(net_buf_add(buf, len));
    memcpy(dat, &brcm_patchram_buf[offset], len);
    offset += len;

    err = bt_hci_cmd_send_sync(opcode, buf, nullptr);
    net_buf_unref(buf);
    if (err) {
      printf("Sending patchram packet failed: %d\r\n", err);
      return;
    }
  }
  // Sleep to let the patched firmware execute.
  vTaskDelay(pdMS_TO_TICKS(200));

  if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
    settings_load();
  }

  if (g_init_cb) {
    g_init_cb(err);
  }

  coralmicro::MutexLock lock(g_ble_scan_mtx);
  g_bt_initialized = true;
}

void InitEdgefastBluetooth(bt_ready_cb_t cb) {
  g_ble_scan_mtx = xSemaphoreCreateMutex();
  CHECK(g_ble_scan_mtx);
  if (coralmicro::LfsReadFile(
          "/third_party/cyw-bt-patch/BCM4345C0_003.001.025.0144.0266.1MW.hcd",
          brcm_patchram_buf, brcm_patch_ram_length) != brcm_patch_ram_length) {
    printf("Reading patchram failed\r\n");
    assert(false);
  }
  wiced_init();
  coralmicro::GpioSet(coralmicro::Gpio::kBtDevWake, false);
  ble_pwr_on();
  g_init_cb = cb;
  int err = bt_enable(bt_ready_internal);
  if (err) {
    printf("bt_enable failed(%d)\r\n", err);
  }
}

void scan_cb(const bt_addr_le_t* addr, int8_t rssi, uint8_t adv_type,
             struct net_buf_simple* buf) {
  char addr_s[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(addr, addr_s, sizeof(addr_s));

  coralmicro::MutexLock lock(g_ble_scan_mtx);
  if (g_scan_results.size() < g_max_num_results && strlen(addr_s) > 0) {
    g_scan_results.emplace_back(addr_s);
  }
}

bool BluetoothReady() { return g_bt_initialized; }

bool BluetoothAdvertise() {
  {
    coralmicro::MutexLock lock(g_ble_scan_mtx);
    if (!g_bt_initialized) {
      printf("Bluetooth is being initialized.\r\n");
      return false;
    }
  }
  char addr_s[BT_ADDR_LE_STR_LEN];
  bt_addr_le_t addr = {0};
  size_t count = 1;
  const auto& params = BT_LE_ADV_NCONN_IDENTITY;
  if (auto err =
          bt_le_adv_start(params, kAdvertiseData, ARRAY_SIZE(kAdvertiseData),
                          kScanResponseData, ARRAY_SIZE(kScanResponseData));
      err) {
    printf("failed to start advertising\r\n");
    return false;
  }

  bt_id_get(&addr, &count);
  bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

  printf("Beacon started, advertising as %s\r\n", addr_s);
  return true;
}

std::optional<std::vector<std::string>> BluetoothScan(
    int max_num_of_results, unsigned int scan_period_ms) {
  {
    coralmicro::MutexLock lock(g_ble_scan_mtx);
    if (!g_bt_initialized) {
      printf("Bluetooth is being initialized.\r\n");
      return std::nullopt;
    }
    g_scan_results.clear();
    g_max_num_results = max_num_of_results;
  }
  const struct bt_le_scan_param scan_param = {
      .type = BT_HCI_LE_SCAN_ACTIVE,
      .options = BT_LE_SCAN_OPT_NONE,
      .interval = 0x0100,
      .window = 0x0010,
  };
  if (auto err = bt_le_scan_start(&scan_param, scan_cb); err) {
    printf("Starting scanning failed (err %d)\r\n", err);
    return std::nullopt;
  }
  vTaskDelay(pdMS_TO_TICKS(scan_period_ms));
  bt_le_scan_stop();
  return g_scan_results;
}
