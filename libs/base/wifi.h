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

#ifndef LIBS_BASE_WIFI_H_
#define LIBS_BASE_WIFI_H_

#include <optional>
#include <vector>

#include "libs/base/utils.h"
extern "C" {
#include "libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

namespace coral::micro {

inline constexpr int kDefaultRetryCount{5};

// Represents the Wi-Fi antenna.
enum class WiFiAntenna {
    // Internal built in Wi-Fi antenna.
    kInternal = 0,
    // External custom Wi-Fi antenna.
    kExternal = 1,
};

// Turns on the Wi-Fi module.
//
// @return True if successfully turned on; false otherwise.
bool TurnOnWiFi();

// Turns off the Wi-Fi module.
//
// @return True if successfully turned off; false otherwise.
bool TurnOffWiFi();

// Checks if the board is connected to a Wi-Fi network.
//
// @return True if it is connected; false otherwise.
bool WiFiIsConnected();

// Connects to a Wi-Fi network.
//
// @param network_params A pointer to a `WIFINetworkParams_t` that contains
// information of the ssid such as name, password, and security type. See:
// https://aws.github.io/amazon-freertos/202107.00/html1/struct_w_i_f_i_network_params__t.html
// @param retry_count The max number of connection attempts. Default is 5.
// @return True if successfully connected to Wi-Fi; false otherwise.
bool ConnectWiFi(const WIFINetworkParams_t* network_params,
                 int retry_count = kDefaultRetryCount);

// Connects to a Wi-Fi network with the given network name and password.
//
// @param ssid The network name.
// @param psk The password for the ssid.
// @param retry_count The max number of connection attempts. Default is 5.
// @return True if successfully connected to Wi-Fi; false otherwise.
bool ConnectWiFi(const char* ssid, const char* psk,
                 int retry_count = kDefaultRetryCount);

// Connects to the Wi-Fi network that's saved on the device.
//
// Internally, this API reads the stored ssid and password using
// `utils::GetWiFiSSID()` and `utils::GetWiFiPSK()`, which could both be set
// either during flash with the `--wifi_ssid` and `--wifi_psk` flags or with a
// direct call to `utils::SetWiFiSSID()` and `utils::SetWiFiPSK()`.
//
// @param retry_count The max number of connection attempts. Default is 5.
// @return True if successfully connected to Wi-Fi; false otherwise.
bool ConnectWiFi(int retry_count = kDefaultRetryCount);

// Disconnects from the Wi-Fi network.
//
// @param retry_count The max number of disconnect attempts. Default is 5.
// @return True if Wi-Fi is successfully disconnected; false otherwise.
bool DisconnectWiFi(int retry_count = kDefaultRetryCount);

// Scans for Wi-Fi networks.
//
// @param max_results The max number of networks to look for.
// This function may return fewer than this number but never more.
// Default is 255 which is the maximum for uint8_t.
// @return A vector of `WIFIScanResult_t` which contains info like name,
// security type, etc. See:
// https://aws.github.io/amazon-freertos/202107.00/html1/struct_w_i_f_i_scan_result__t.html
std::vector<WIFIScanResult_t> ScanWiFi(
    uint8_t max_results = std::numeric_limits<uint8_t>::max());

// Gets the device's Wi-Fi IP address.
//
// @return A string representing the IPv4 IP address or `std::nullopt` on
// failure.
std::optional<std::string> GetWiFiIp();

// Sets which Wi-Fi antenna type to use (internal or external).
//
// @param antenna The type of antenna to use.
// @return True if the Wi-Fi antenna was enabled successfully; false otherwise.
bool SetWiFiAntenna(WiFiAntenna antenna);

}  // namespace coral::micro
#endif  // LIBS_BASE_WIFI_H_
