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

#ifndef LIBS_NXP_RT1176_SDK_EDGEFAST_BLUETOOTH_EDGEFAST_BLUETOOTH_H_
#define LIBS_NXP_RT1176_SDK_EDGEFAST_BLUETOOTH_EDGEFAST_BLUETOOTH_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>
/* clang-format off */
#include "third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/include/toolchain.h"
#include "third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/include/bluetooth/bluetooth.h"
#include "third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/include/bluetooth/hci.h"
/* clang-format on */

// Initializes Bluetooth for the Dev Board Micro.
//
// Requires the Coral Wireless Add-on (or similar add-on board).
// This is a wrapper for `bt_enable()` and performs other board-specific setup.
// Note: This function is non-blocking and bluetooth could be uninitialized
// after return. Please check `BluetoothReady()` to make sure it is in fact
// initialized before using other functions.
// @param cb  Callback to notify when Bluetooth is enabled or fails to enable.
//
void InitEdgefastBluetooth(bt_ready_cb_t cb);

// Check if bluetooth is ready.
//
// @return true if bluetooth is ready, false if it is not.
bool BluetoothReady();

// Start bluetooth advertising.
// @return true id advertised successfully, false if advertise failed.
bool BluetoothAdvertise();

// Performs bluetooth scan.
//
// @param max_results Number of scan results.
// @param scan_period_ms Time in ms to scan for devices.
// @return An array of bluetooth ids.
std::optional<std::vector<std::string>> BluetoothScan(
    int max_results, unsigned int scan_period_ms);

#endif  // LIBS_NXP_RT1176_SDK_EDGEFAST_BLUETOOTH_EDGEFAST_BLUETOOTH_H_
