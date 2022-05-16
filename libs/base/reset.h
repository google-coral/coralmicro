#ifndef LIBS_BASE_RESET_H_
#define LIBS_BASE_RESET_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_soc_src.h"

namespace coral::micro {

struct ResetStats {
    uint32_t reset_reason;
    uint32_t watchdog_resets;
    uint32_t lockup_resets;
};

inline constexpr src_general_purpose_register_index_t kRebootedByWatchdogCount =
    kSRC_GeneralPurposeRegister13;
inline constexpr src_general_purpose_register_index_t kRebootedByLockupCount =
    kSRC_GeneralPurposeRegister14;

void ResetToBootloader();
void ResetToFlash();
void StoreResetReason();
const ResetStats GetResetStats();

}  // namespace coral::micro

#endif  // LIBS_BASE_RESET_H_
