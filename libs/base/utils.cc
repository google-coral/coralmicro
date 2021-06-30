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

}  // namespace utils
}  // namespace valiant