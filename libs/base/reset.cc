/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libs/base/reset.h"

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_romapi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_soc_src.h"

namespace coralmicro {
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

void ResetStoreStats() {
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
    lockup_count = SRC_GetGeneralPurposeRegister(SRC, kRebootedByLockupCount);
    lockup_count++;
    SRC_SetGeneralPurposeRegister(SRC, kRebootedByLockupCount, lockup_count);
    reset_stats.lockup_resets = lockup_count;
  }
}

ResetStats ResetGetStats() { return reset_stats; }
}  // namespace coralmicro
