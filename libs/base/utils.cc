#include "libs/base/utils.h"

#include "libs/base/filesystem.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/ip_addr.h"

#include <vector>

namespace valiant {
namespace utils {

uint64_t GetUniqueID() {
    uint32_t fuse_val_hi, fuse_val_lo;
    fuse_val_lo = OCOTP->FUSEN[16].FUSE;
    fuse_val_hi = OCOTP->FUSEN[17].FUSE;
    return ((static_cast<uint64_t>(fuse_val_hi) << 32) | fuse_val_lo);
}

std::string GetSerialNumber() {
    uint64_t id = valiant::utils::GetUniqueID();
    char serial[17];  // 16 hex characters + \0
    snprintf(serial, sizeof(serial), "%08lx%08lx",
             static_cast<uint32_t>(id >> 32), static_cast<uint32_t>(id));
    return std::string(serial);
}

MacAddress GetMacAddress() {
    uint32_t fuse_val_hi, fuse_val_lo;
    fuse_val_lo = OCOTP->FUSEN[FUSE_ADDRESS_TO_OCOTP_INDEX(MAC1_ADDR_LO)].FUSE;
    fuse_val_hi = OCOTP->FUSEN[FUSE_ADDRESS_TO_OCOTP_INDEX(MAC1_ADDR_HI)].FUSE & 0xFFFF;
    uint8_t a = (fuse_val_hi >> 8) & 0xFF;
    uint8_t b = (fuse_val_hi) & 0xFF;
    uint8_t c = (fuse_val_lo >> 24) & 0xFF;
    uint8_t d = (fuse_val_lo >> 16) & 0xFF;
    uint8_t e = (fuse_val_lo >> 8) & 0xFF;
    uint8_t f = (fuse_val_lo) & 0xFF;
    MacAddress m(a, b, c, d, e, f);
    return m;
}

bool GetUSBIPAddress(ip4_addr_t* usb_ip_out) {
    std::string usb_ip;
    if (!GetUSBIPAddress(&usb_ip)) return false;
    if (!ipaddr_aton(usb_ip.c_str(), usb_ip_out)) return false;
    return true;
}

bool GetUSBIPAddress(std::string* usb_ip_out) {
    return valiant::filesystem::ReadFile("/usb_ip_address", usb_ip_out);
}

bool GetWifiSSID(std::string* wifi_ssid_out) {
    return valiant::filesystem::ReadFile("/wifi_ssid", wifi_ssid_out);
}

bool GetWifiPSK(std::string* wifi_psk_out) {
    return valiant::filesystem::ReadFile("/wifi_psk", wifi_psk_out);
}

}  // namespace utils
}  // namespace valiant
