#ifndef LIBS_BASE_WIFI_H_
#define LIBS_BASE_WIFI_H_

#include "libs/base/utils.h"

extern "C" {
#include "libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

namespace coral::micro {

bool TurnOnWiFi();
bool TurnOffWiFi();

bool ConnectToWifi(const WIFINetworkParams_t* network_params, int retry_count);
bool ConnectToWiFi(const char* ssid, const char* psk, int retry_count);
bool ConnectToWiFi();

}  // namespace coral::micro

#endif  // LIBS_BASE_WIFI_H_
