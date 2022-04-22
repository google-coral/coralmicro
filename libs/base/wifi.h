#ifndef _LIBS_BASE_WIFI_H_
#define _LIBS_BASE_WIFI_H_

namespace valiant {

bool TurnOnWiFi();
bool TurnOffWiFi();

bool ConnectToWiFi(const char* ssid, const char* psk, int retry_count);
bool ConnectToWiFi();

}  // namespace valiant

#endif  // _LIBS_BASE_WIFI_H_
