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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_SCALAR_CORE_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_SCALAR_CORE_CSR_OFFSETS_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets that will be dumped as part of the
// driver bug report for scalar core. Members are intentionally named to match
// the GCSR register names.
struct DebugScalarCoreCsrOffsets {
  uint64_t topology;
  uint64_t scMemoryCapacity;
  uint64_t tileMemoryCapacity;
  uint64_t scalarCoreRunControl;
  uint64_t scalarCoreBreakPoint;
  uint64_t currentPc;
  uint64_t executeControl;
  uint64_t scMemoryAccess;
  uint64_t scMemoryData;
  uint64_t SyncCounter_AVDATA_POP;
  uint64_t SyncCounter_PARAMETER_POP;
  uint64_t SyncCounter_AVDATA_INFEED;
  uint64_t SyncCounter_PARAMETER_INFEED;
  uint64_t SyncCounter_SCALAR_INFEED;
  uint64_t SyncCounter_PRODUCER_A;
  uint64_t SyncCounter_PRODUCER_B;
  uint64_t SyncCounter_RING_OUTFEED;
  uint64_t avDataPopRunControl;
  uint64_t avDataPopBreakPoint;
  uint64_t avDataPopRunStatus;
  uint64_t avDataPopOverwriteMode;
  uint64_t avDataPopEnableTracing;
  uint64_t avDataPopStartCycle;
  uint64_t avDataPopEndCycle;
  uint64_t avDataPopProgramCounter;
  uint64_t parameterPopRunControl;
  uint64_t parameterPopBreakPoint;
  uint64_t parameterPopRunStatus;
  uint64_t parameterPopOverwriteMode;
  uint64_t parameterPopEnableTracing;
  uint64_t parameterPopStartCycle;
  uint64_t parameterPopEndCycle;
  uint64_t parameterPopProgramCounter;
  uint64_t infeedRunControl;
  uint64_t infeedRunStatus;
  uint64_t infeedBreakPoint;
  uint64_t infeedOverwriteMode;
  uint64_t infeedEnableTracing;
  uint64_t infeedStartCycle;
  uint64_t infeedEndCycle;
  uint64_t infeedProgramCounter;
  uint64_t outfeedRunControl;
  uint64_t outfeedRunStatus;
  uint64_t outfeedBreakPoint;
  uint64_t outfeedOverwriteMode;
  uint64_t outfeedEnableTracing;
  uint64_t outfeedStartCycle;
  uint64_t outfeedEndCycle;
  uint64_t outfeedProgramCounter;
  uint64_t scalarCoreRunStatus;
  uint64_t Error_ScalarCore;
  uint64_t Error_Mask_ScalarCore;
  uint64_t Error_Force_ScalarCore;
  uint64_t Error_Timestamp_ScalarCore;
  uint64_t Error_Info_ScalarCore;
  uint64_t Timeout;
  uint64_t avDataPopTtuStateRegFile;
  uint64_t avDataPopTrace;
  uint64_t parameterPopTtuStateRegFile;
  uint64_t parameterPopTrace;
  uint64_t infeedTtuStateRegFile;
  uint64_t outfeedTtuStateRegFile;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_SCALAR_CORE_CSR_OFFSETS_H_
