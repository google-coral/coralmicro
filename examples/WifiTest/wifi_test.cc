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

#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>
#include <cstring>

extern "C" {
#include "libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

extern "C" void app_main(void *param) {
    printf("WifiTest begin\r\n");
    WIFIReturnCode_t xWifiStatus;
    const uint8_t ucNumNetworks = 50;
    WIFIScanResult_t xScanResults[ucNumNetworks] = {0};

    // Uncomment me to use the external antenna.
    // coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kAntennaSelect, true);

    do {
        xWifiStatus = WIFI_On();
        if (xWifiStatus != eWiFiSuccess) {
            printf("Failed to turn on WiFi\r\n");
            break;
        }
        xWifiStatus = WIFI_Scan(xScanResults, ucNumNetworks);
        if (xWifiStatus != eWiFiSuccess) {
            printf("Scan failed\r\n");
            break;
        }

        for (int i = 0; i < ucNumNetworks; ++i) {
            printf("cSSID: %s RSSI: %d\r\n", xScanResults[i].cSSID, xScanResults[i].cRSSI);
        }

        WIFINetworkParams_t xNetworkParams;
        xNetworkParams.pcSSID = WIFI_SSID;
        xNetworkParams.ucSSIDLength = strlen(WIFI_SSID);
        xNetworkParams.pcPassword = WIFI_PSK;
        xNetworkParams.ucPasswordLength = strlen(WIFI_PSK);
        xNetworkParams.xSecurity = strlen(WIFI_PSK) == 0 ? eWiFiSecurityOpen : eWiFiSecurityWPA2;
        printf("Attempting to connect to %s\r\n", WIFI_SSID);
        xWifiStatus = WIFI_ConnectAP(&xNetworkParams);
        if (xWifiStatus != eWiFiSuccess) {
            printf("Failed to connect to AP\r\n");
            break;
        }

        uint8_t ucIPAddr[4];

        xWifiStatus = WIFI_GetIP(ucIPAddr);
        if (xWifiStatus != eWiFiSuccess) {
            printf("Failed to get our IP\r\n");
        }
        printf("Our IP -> %d.%d.%d.%d\r\n", ucIPAddr[0], ucIPAddr[1], ucIPAddr[2], ucIPAddr[3]);

        xWifiStatus = WIFI_GetHostIP("google.com", ucIPAddr);
        if (xWifiStatus != eWiFiSuccess) {
            printf("Failed to resolve google.com\r\n");
            break;
        }

        printf("google.com -> %d.%d.%d.%d\r\n", ucIPAddr[0], ucIPAddr[1], ucIPAddr[2], ucIPAddr[3]);
    } while (false);
    WIFI_Off();
    printf("WifiTest end\r\n");
    vTaskSuspend(nullptr);
}
