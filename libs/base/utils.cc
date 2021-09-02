#include "libs/base/utils.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.h"

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

}  // namespace utils
}  // namespace valiant
