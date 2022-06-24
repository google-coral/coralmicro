#include "libs/base/reset.h"

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_romapi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_soc_src.h"

namespace coral::micro {
namespace {
constexpr src_general_purpose_register_index_t kRebootedByWatchdogCount =
    kSRC_GeneralPurposeRegister13;
constexpr src_general_purpose_register_index_t kRebootedByLockupCount =
    kSRC_GeneralPurposeRegister14;
ResetStats reset_stats;
}  // namespace

void ResetToBootloader() {
    uint32_t boot_arg = 0xeb100000;
    ROM_API_Init();
    ROM_RunBootloader(&boot_arg);
}

void ResetToFlash() { SNVS->LPCR |= SNVS_LPCR_TOP_MASK; }

void StoreResetReason() {
    uint32_t watchdog_trip_count, lockup_count;
    reset_stats.reset_reason = SRC_GetResetStatusFlags(SRC);
    SRC_ClearGlobalSystemResetStatus(SRC, 0xFFFFFFFF);
    if (reset_stats.reset_reason & kSRC_M7CoreWdogResetFlag) {
        watchdog_trip_count =
            SRC_GetGeneralPurposeRegister(SRC, kRebootedByWatchdogCount);
        watchdog_trip_count++;
        SRC_SetGeneralPurposeRegister(SRC, kRebootedByWatchdogCount,
                                      watchdog_trip_count);
        reset_stats.watchdog_resets = watchdog_trip_count;
    }
    if (reset_stats.reset_reason & kSRC_M7CoreM7LockUpResetFlag) {
        lockup_count =
            SRC_GetGeneralPurposeRegister(SRC, kRebootedByLockupCount);
        lockup_count++;
        SRC_SetGeneralPurposeRegister(SRC, kRebootedByLockupCount,
                                      lockup_count);
        reset_stats.lockup_resets = lockup_count;
    }
}

ResetStats GetResetStats() { return reset_stats; }
}  // namespace coral::micro
