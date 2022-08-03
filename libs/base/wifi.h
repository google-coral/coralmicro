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

#ifndef LIBS_BASE_WIFI_H_
#define LIBS_BASE_WIFI_H_

#include <optional>
#include <vector>

#include "libs/base/utils.h"
extern "C" {
#include "third_party/modified/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

namespace coralmicro {

inline constexpr int kDefaultRetryCount{5};

// Represents the Wi-Fi antenna.
enum class WiFiAntenna {
  // Internal built in Wi-Fi antenna.
  kInternal,
  // External custom Wi-Fi antenna.
  kExternal,
};

// Gets the Wi-Fi SSID that is stored in flash memory.
// @param wifi_ssid_out A pointer to a string in which to store the SSID.
// @returns True if the SSID was successfully retrieved; false otherwise.
bool WiFiGetDefaultSsid(std::string* wifi_ssid_out);

// Sets the Wi-Fi SSID in flash memory.
// @param wifi_ssid A pointer to a string containing the SSID.
// @returns True if the SSID was successfully stored; false otherwise.
bool WiFiSetDefaultSsid(const std::string& wifi_ssid);

// Gets the Wi-Fi key that is stored in flash memory.
// @param wifi_ssid_out A pointer to a string in which to store the SSID.
// @returns True if the SSID was successfully retrieved; false otherwise.
bool WiFiGetDefaultPsk(std::string* wifi_psk_out);

// Sets the Wi-Fi key in flash memory.
// @param wifi_psk A pointer to a string containing the key.
// @returns True if the key was successfully stored; false otherwise.
bool WiFiSetDefaultPsk(const std::string& wifi_psk);

// Turns on the Wi-Fi module.
//
// @param default_iface True sets Wi-Fi as the default network interface.
// @return True if successfully turned on; false otherwise.
bool WiFiTurnOn(bool default_iface);

// Turns off the Wi-Fi module.
//
// @return True if successfully turned off; false otherwise.
bool WiFiTurnOff();

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
bool WiFiConnect(const WIFINetworkParams_t& network_params,
                 int retry_count = kDefaultRetryCount);

// Connects to a Wi-Fi network with the given network name and password.
//
// @param ssid The network name.
// @param psk The password for the ssid.
// @param retry_count The max number of connection attempts. Default is 5.
// @return True if successfully connected to Wi-Fi; false otherwise.
bool WiFiConnect(const char* ssid, const char* psk,
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
bool WiFiConnect(int retry_count = kDefaultRetryCount);

// Disconnects from the Wi-Fi network.
//
// @param retry_count The max number of disconnect attempts. Default is 5.
// @return True if Wi-Fi is successfully disconnected; false otherwise.
bool WiFiDisconnect(int retry_count = kDefaultRetryCount);

// Scans for Wi-Fi networks.
//
// @return A vector of `WIFIScanResult_t` which contains info like name,
// security type, etc. See:
// https://aws.github.io/amazon-freertos/202107.00/html1/struct_w_i_f_i_scan_result__t.html
std::vector<WIFIScanResult_t> WiFiScan();

// Gets the device's Wi-Fi IP address.
//
// @return A string representing the IPv4 IP address or `std::nullopt` on
// failure.
std::optional<std::string> WiFiGetIp();

// Gets the device's Wi-Fi MAC address.
//
// @return Byte array containing the MAC address or `std::nullopt` on failure.
std::optional<std::array<uint8_t, 6>> WiFiGetMac();

// Gets the BSSID (MAC address of connected Access Point).
//
// @return Byte array containing the MAC address or `std::nullopt` on
// failure.
std::optional<std::array<uint8_t, 6>> WiFiGetBssid();

// Gets the RSSI of the connected access point.
//
// @return Signal strength in dBm or `std::nullopt` on failure.
std::optional<int32_t> WiFiGetRssi();

// Sets which Wi-Fi antenna type to use (internal or external).
//
// @param antenna The type of antenna to use.
void WiFiSetAntenna(WiFiAntenna antenna);

}  // namespace coralmicro
#endif  // LIBS_BASE_WIFI_H_
