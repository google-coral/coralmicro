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

#ifndef LIBS_BASE_RESET_H_
#define LIBS_BASE_RESET_H_

#include <cstdint>

namespace coralmicro {

// Represents reset stats.
struct ResetStats {
  // Reset reason, could hold kSRC_M7CoreWdogResetFlag or
  // kSRC_M7CoreM7LockUpResetFlag.
  uint32_t reset_reason;
  // Number of watchdog resets.
  uint32_t watchdog_resets;
  // Number of lockup resets.
  uint32_t lockup_resets;
};

// Reset the board to bootloader mode.
void ResetToBootloader();

// Reset the board to flash mode.
void ResetToFlash();

// Stores the current reset stats.
void ResetStoreStats();

// Gets the current reset stats.
ResetStats ResetGetStats();

}  // namespace coralmicro

#endif  // LIBS_BASE_RESET_H_
