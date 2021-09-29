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

bool GetUSBIPAddress(ip4_addr_t* addr) {
    constexpr const char *usb_ip_address_path = "/usb_ip_address";
    if (!addr) {
        return false;
    }

    size_t usb_ip_address_len = valiant::filesystem::Size(usb_ip_address_path);
    if (usb_ip_address_len < 0) {
        return false;
    }

    std::vector<char> usb_ip_address(usb_ip_address_len + 1, 0);
    if (!valiant::filesystem::ReadToMemory(usb_ip_address_path, reinterpret_cast<uint8_t*>(usb_ip_address.data()), &usb_ip_address_len)) {
        return false;
    }

    if (!ipaddr_aton(usb_ip_address.data(), addr)) {
        return false;
    }
    return true;
}

}  // namespace utils
}  // namespace valiant
