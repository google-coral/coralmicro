#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_romapi.h"

namespace coral::micro {

void ResetToBootloader() {
    uint32_t boot_arg = 0xeb100000;
    ROM_API_Init();
    ROM_RunBootloader(&boot_arg);
}

}  // namespace coral::micro
