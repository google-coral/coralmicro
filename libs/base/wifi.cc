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

#include "libs/base/wifi.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>
#include <string>

#include "libs/base/check.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/ntp.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/wwd_wifi.h"

namespace coralmicro {

bool WiFiGetDefaultSsid(std::string* wifi_ssid_out) {
  return LfsReadFile("/wifi_ssid", wifi_ssid_out);
}

bool WiFiSetDefaultSsid(const std::string& wifi_ssid) {
  return LfsWriteFile("/wifi_ssid", wifi_ssid);
}

bool WiFiGetDefaultPsk(std::string* wifi_psk_out) {
  return LfsReadFile("/wifi_psk", wifi_psk_out);
}

bool WiFiSetDefaultPsk(const std::string& wifi_psk) {
  return LfsWriteFile("/wifi_psk", wifi_psk);
}

bool WiFiTurnOn(bool default_iface) {
  return WIFI_On(default_iface) == eWiFiSuccess;
}

bool WiFiTurnOff() { return WIFI_Off() == eWiFiSuccess; }

bool WiFiIsConnected() { return WIFI_IsConnected(); }

bool WiFiConnect(const WIFINetworkParams_t& network_params, int retry_count) {
  while (retry_count != 0) {
    if (WIFI_ConnectAP(&network_params) == eWiFiSuccess) {
      NtpInit();
      return true;
    }
    --retry_count;
  }

  printf("Failed to connect to %s\r\n", network_params.pcSSID);
  return false;
}

bool WiFiConnect(const char* ssid, const char* psk, int retry_count) {
  assert(ssid);
  assert(retry_count > 0);

  WIFINetworkParams_t params;
  params.pcSSID = ssid;
  params.ucSSIDLength = std::strlen(ssid);
  if (psk != nullptr) {
    params.pcPassword = psk;
    params.ucPasswordLength = std::strlen(psk);
    params.xSecurity = eWiFiSecurityWPA2;
  } else {
    params.pcPassword = "";
    params.ucPasswordLength = 0;
    params.xSecurity = eWiFiSecurityOpen;
  }
  return WiFiConnect(params, retry_count);
}

bool WiFiConnect(int retry_count) {
  std::string wifi_ssid;
  if (!WiFiGetDefaultSsid(&wifi_ssid)) {
    printf("No Wi-Fi SSID provided\r\n");
    return false;
  }

  std::string wifi_psk;
  bool have_psk = WiFiGetDefaultPsk(&wifi_psk);
  return WiFiConnect(wifi_ssid.c_str(), have_psk ? wifi_psk.c_str() : nullptr,
                     retry_count);
}

bool WiFiDisconnect(int retry_count) {
  if (WIFI_IsConnected()) {
    while (retry_count != 0) {
      if (WIFI_Disconnect() == eWiFiSuccess) return true;
      retry_count--;
    }
    return false;
  }
  return true;
}

std::vector<WIFIScanResult_t> WiFiScan() {
  std::vector<WIFIScanResult_t> results(std::numeric_limits<uint8_t>::max());
  if (WIFI_Scan(results.data(), results.size()) != eWiFiSuccess) return {};

  results.erase(std::remove_if(std::begin(results), std::end(results),
                               [](const WIFIScanResult_t& result) {
                                 return result.cSSID[0] == '\0';
                               }),
                std::end(results));
  return results;
}

std::optional<std::string> WiFiGetIp() {
  if (!WiFiIsConnected()) {
    return std::nullopt;
  }
  uint8_t ip[4];
  if (WIFI_GetIP(ip) != eWiFiSuccess) {
    return std::nullopt;
  }
  return std::to_string(ip[0]) + '.' + std::to_string(ip[1]) + '.' +
         std::to_string(ip[2]) + '.' + std::to_string(ip[3]);
}

std::optional<std::array<uint8_t, 6>> WiFiGetMac() {
  std::array<uint8_t, 6> mac;
  if (WIFI_GetMAC(mac.data()) != eWiFiSuccess) {
    return std::nullopt;
  }

  return mac;
}

std::optional<std::array<uint8_t, 6>> WiFiGetBssid() {
  if (!WiFiIsConnected()) {
    return std::nullopt;
  }
  wiced_mac_t bssid;
  if (wwd_wifi_get_bssid(&bssid) != WWD_SUCCESS) {
    return std::nullopt;
  }
  std::array<uint8_t, 6> ret_bssid;
  memcpy(ret_bssid.data(), bssid.octet, 6);
  return ret_bssid;
}

std::optional<int32_t> WiFiGetRssi() {
  if (!WiFiIsConnected()) {
    return std::nullopt;
  }
  int32_t rssi;
  if (wwd_wifi_get_rssi(&rssi) != WWD_SUCCESS) {
    return std::nullopt;
  }
  return rssi;
}

void WiFiSetAntenna(WiFiAntenna antenna) {
  switch (antenna) {
    case WiFiAntenna::kInternal:
      GpioSet(Gpio::kAntennaSelect, false);
      break;
    case WiFiAntenna::kExternal:
      GpioSet(Gpio::kAntennaSelect, true);
      break;
    default:
      CHECK(!"Invalid antenna value");
  }
}

}  // namespace coralmicro
