#ifndef _LIBS_BASE_UTILS_H_
#define _LIBS_BASE_UTILS_H_

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/ip_addr.h"
#include <cstdint>
#include <string>

#define FUSE_ADDRESS_TO_OCOTP_INDEX(fuse) ((fuse - 0x800) >> 4)
#define MAC1_ADDR_LO (0xA80)
#define MAC1_ADDR_HI (0xA90)

namespace valiant {

struct MacAddress {
    MacAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) :
        a(a), b(b), c(c), d(d), e(e), f(f) {}
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;
};

namespace utils {
uint64_t GetUniqueID();
MacAddress GetMacAddress();
bool GetUSBIPAddress(ip4_addr_t* addr);
bool GetWifiSSID(std::string* wifi_ssid_out);
bool GetWifiPSK(std::string* wifi_psk_out);

inline uint64_t StrHash(const char* s) {
    uint64_t h = 0;
    while (*s) {
        h = h * 101 + (uint64_t) *(s++);
    }
    return h;
}

}  // namespace utils
}  // namespace valiant

#endif  // _LIBS_BASE_UTILS_H_
