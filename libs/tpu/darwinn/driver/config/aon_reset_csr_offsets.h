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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_AON_RESET_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_AON_RESET_CSR_OFFSETS_H_

#include "libs/tpu/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for aon reset. Members are
// intentionally named to match the GCSR register names.
struct AonResetCsrOffsets {
  // Resets CB.
  uint64 resetReg;

  // Clock enable register.
  uint64 clockEnableReg;

  // Shutdown registers.
  uint64 logicShutdownReg;
  uint64 logicShutdownPreReg;
  uint64 logicShutdownAllReg;
  uint64 memoryShutdownReg;
  uint64 memoryShutdownAckReg;
  uint64 tileMemoryRetnReg;

  // Clamp register.
  uint64 clampEnableReg;

  // Quiesce register.
  uint64 forceQuiesceReg;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_AON_RESET_CSR_OFFSETS_H_
