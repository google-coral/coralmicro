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

static bool GetFileDataAsCStr(const char *path, std::vector<char> *out, size_t *out_len) {
    size_t len = valiant::filesystem::Size(path);
    if (len < 0) {
        return false;
    }

    out->resize(len + 1);
    std::fill(out->begin(), out->end(), 0);
    if (!valiant::filesystem::ReadToMemory(path, reinterpret_cast<uint8_t*>(out->data()), &len)) {
        return false;
    }

    if (out_len) {
        *out_len = len;
    }

    return true;
}

bool GetUSBIPAddress(ip4_addr_t* addr) {
    constexpr const char *usb_ip_address_path = "/usb_ip_address";
    if (!addr) {
        return false;
    }

    std::vector<char> usb_ip_address;
    if (!GetFileDataAsCStr(usb_ip_address_path, &usb_ip_address, nullptr)) {
        return false;
    }

    if (!ipaddr_aton(usb_ip_address.data(), addr)) {
        return false;
    }
    return true;
}

bool GetWifiSSID(std::string* wifi_ssid_out) {
    constexpr const char *wifi_ssid_path = "/wifi_ssid";
    std::vector<char> wifi_ssid_chr;
    size_t len;
    if (!GetFileDataAsCStr(wifi_ssid_path, &wifi_ssid_chr, &len)) {
        return false;
    }

    wifi_ssid_out->erase();
    wifi_ssid_out->append(wifi_ssid_chr.data(), len);
    return true;
}

bool GetWifiPSK(std::string* wifi_psk_out) {
    constexpr const char *wifi_psk_path = "/wifi_psk";
    std::vector<char> wifi_psk_chr;
    size_t len;
    if (!GetFileDataAsCStr(wifi_psk_path, &wifi_psk_chr, &len)) {
        return false;
    }

    wifi_psk_out->erase();
    wifi_psk_out->append(wifi_psk_chr.data(), len);
    return true;
}

std::size_t Base64Size(std::size_t data_size) {
        return (((data_size * 4) / 3) + 4 - 1) & -4;;
}

}  // namespace utils
}  // namespace valiant
