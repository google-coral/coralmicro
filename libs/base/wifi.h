#ifndef LIBS_BASE_WIFI_H_
#define LIBS_BASE_WIFI_H_

namespace coral::micro {

bool TurnOnWiFi();
bool TurnOffWiFi();

bool ConnectToWiFi(const char* ssid, const char* psk, int retry_count);
bool ConnectToWiFi();

}  // namespace coral::micro

#endif  // LIBS_BASE_WIFI_H_
