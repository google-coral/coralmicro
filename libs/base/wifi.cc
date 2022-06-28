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

#include <cassert>
#include <cstring>
#include <string>

#include "libs/base/gpio.h"

namespace coral::micro {

bool TurnOnWiFi() { return WIFI_On() == eWiFiSuccess; }

bool TurnOffWiFi() { return WIFI_Off() == eWiFiSuccess; }

bool WiFiIsConnected() { return WIFI_IsConnected(); }

bool ConnectWiFi(const WIFINetworkParams_t* network_params, int retry_count) {
    while (retry_count != 0) {
        if (WIFI_ConnectAP(network_params) == eWiFiSuccess) return true;
        --retry_count;
    }

    printf("Failed to connect to %s\r\n", network_params->pcSSID);
    return false;
}

bool ConnectWiFi(const char* ssid, const char* psk, int retry_count) {
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
    return ConnectWiFi(&params, retry_count);
}

bool ConnectWiFi(int retry_count) {
    std::string wifi_ssid;
    if (!coral::micro::utils::GetWiFiSSID(&wifi_ssid)) {
        printf("No Wi-Fi SSID provided\r\n");
        return false;
    }

    std::string wifi_psk;
    bool have_psk = coral::micro::utils::GetWiFiPSK(&wifi_psk);
    return ConnectWiFi(wifi_ssid.c_str(), have_psk ? wifi_psk.c_str() : nullptr,
                       retry_count);
}

bool DisconnectWiFi(int retry_count) {
    if (WIFI_IsConnected()) {
        while (retry_count != 0) {
            if (WIFI_Disconnect() == eWiFiSuccess) return true;
            retry_count--;
        }
        return false;
    }
    return true;
}

std::vector<WIFIScanResult_t> ScanWiFi(uint8_t max_results) {
    std::vector<WIFIScanResult_t> results{max_results};
    if (WIFI_Scan(results.data(), results.size()) != eWiFiSuccess) {
        return {};
    }
    return results;
}

std::optional<std::string> GetWiFiIp() {
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

bool SetWiFiAntenna(WiFiAntenna antenna) {
    switch (antenna) {
        case WiFiAntenna::kInternal:
            gpio::SetGpio(gpio::Gpio::kAntennaSelect, false);
            return true;
        case WiFiAntenna::kExternal:
            gpio::SetGpio(gpio::Gpio::kAntennaSelect, true);
            return true;
        default:
            return false;
    }
}

}  // namespace coral::micro
