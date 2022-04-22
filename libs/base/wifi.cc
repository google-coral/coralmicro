#include "libs/base/wifi.h"

#include <cassert>
#include <cstring>
#include <string>

#include "libs/base/utils.h"

extern "C" {
#include "libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

namespace valiant {
namespace {
constexpr int kDefaultRetryCount = 5;
}  // namespace

bool TurnOnWiFi() { return WIFI_On() == eWiFiSuccess; }

bool TurnOffWiFi() { return WIFI_Off() == eWiFiSuccess; }

bool ConnectToWiFi(const char* ssid, const char* psk, int retry_count) {
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

    while (retry_count != 0) {
        if (WIFI_ConnectAP(&params) == eWiFiSuccess) return true;
        --retry_count;
    }

    printf("Failed to connect to %s\r\n", ssid);
    return false;
}

bool ConnectToWiFi() {
    std::string wifi_ssid;
    if (!valiant::utils::GetWifiSSID(&wifi_ssid)) {
        printf("No Wi-Fi SSID provided\r\n");
        return false;
    }

    std::string wifi_psk;
    bool have_psk = valiant::utils::GetWifiPSK(&wifi_psk);
    return ConnectToWiFi(wifi_ssid.c_str(),
                         have_psk ? wifi_psk.c_str() : nullptr,
                         kDefaultRetryCount);
}
}  // namespace valiant
