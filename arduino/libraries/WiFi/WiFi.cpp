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

#include "WiFi.h"

#include <cassert>

#include "libs/base/check.h"

namespace coralmicro {
namespace arduino {

namespace {
uint8_t WIFISecurityToArduinoSecurity(WIFISecurity_t security) {
  switch (security) {
    case eWiFiSecurityOpen:
      return ENC_TYPE_NONE;
    case eWiFiSecurityWEP:
      return ENC_TYPE_WEP;
    case eWiFiSecurityWPA:
      return ENC_TYPE_TKIP;
    case eWiFiSecurityWPA2:
    case eWiFiSecurityWPA2_ent:
      return ENC_TYPE_CCMP;
    case eWiFiSecurityNotSupported:
    default:
      return ENC_TYPE_UNKNOWN;
  }
}
}  // namespace

int WiFiClass::begin(const char* ssid) { return begin(ssid, ""); }

int WiFiClass::begin(const char* ssid, const char* passphrase) {
  bool success = coralmicro::WiFiConnect(ssid, passphrase);
  network_status_ = success ? WL_CONNECTED : WL_CONNECT_FAILED;
  if (success) {
    ssid_.reset(nullptr);
    ssid_ = std::make_unique<char[]>(strlen(ssid) + 1);
    memcpy(ssid_.get(), ssid, strlen(ssid) + 1);
  }
  return network_status_;
}

int WiFiClass::disconnect() {
  CHECK(coralmicro::WiFiDisconnect());
  network_status_ = WL_DISCONNECTED;
  ssid_.reset(nullptr);
  return network_status_;
}

char* WiFiClass::SSID() { return ssid_.get(); }

char* WiFiClass::SSID(uint8_t networkItem) {
  CHECK(networkItem < scan_results_.size());
  return scan_results_[networkItem].cSSID;
}

uint8_t* WiFiClass::BSSID(uint8_t* bssid) {
  CHECK(bssid);
  auto bssid_internal = coralmicro::WiFiGetBssid();
  if (!bssid_internal.has_value()) {
    return nullptr;
  }
  memcpy(bssid, bssid_internal.value().data(), bssid_internal.value().size());
  return bssid;
}

int32_t WiFiClass::RSSI() {
  auto rssi = coralmicro::WiFiGetRssi();
  if (!rssi.has_value()) {
    return INT32_MIN;
  }
  return rssi.value();
}

int32_t WiFiClass::RSSI(uint8_t networkItem) {
  CHECK(networkItem < scan_results_.size());
  return scan_results_[networkItem].cRSSI;
}

uint8_t WiFiClass::encryptionType() {
  return WIFISecurityToArduinoSecurity(WIFI_GetSecurity());
}

uint8_t WiFiClass::encryptionType(uint8_t networkItem) {
  CHECK(networkItem < scan_results_.size());
  WIFISecurity_t security = scan_results_[networkItem].xSecurity;
  return WIFISecurityToArduinoSecurity(security);
}

int8_t WiFiClass::scanNetworks() {
  scan_results_ = coralmicro::WiFiScan();
  return static_cast<int8_t>(scan_results_.size());
}

uint8_t WiFiClass::status() { return network_status_; }

uint8_t* WiFiClass::macAddress(uint8_t* mac) {
  auto mac_internal = coralmicro::WiFiGetMac();
  if (!mac_internal.has_value()) {
    return nullptr;
  }
  memcpy(mac, mac_internal.value().data(), mac_internal.value().size());
  return mac;
}

IPAddress WiFiClass::localIP() {
  if (WiFiIsConnected()) {
    uint8_t ip[4];
    if (WIFI_GetIP(ip) == eWiFiSuccess) {
      return IPAddress(ip);
    }
  }
  return IPAddress();
}

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::WiFiClass WiFi;
