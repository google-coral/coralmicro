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

#include "libs/tpu/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets that will be dumped as part of the
// driver bug report for scalar core. Members are intentionally named to match
// the GCSR register names.
struct DebugScalarCoreCsrOffsets {
  uint64 topology;
  uint64 scMemoryCapacity;
  uint64 tileMemoryCapacity;
  uint64 scalarCoreRunControl;
  uint64 scalarCoreBreakPoint;
  uint64 currentPc;
  uint64 executeControl;
  uint64 scMemoryAccess;
  uint64 scMemoryData;
  uint64 SyncCounter_AVDATA_POP;
  uint64 SyncCounter_PARAMETER_POP;
  uint64 SyncCounter_AVDATA_INFEED;
  uint64 SyncCounter_PARAMETER_INFEED;
  uint64 SyncCounter_SCALAR_INFEED;
  uint64 SyncCounter_PRODUCER_A;
  uint64 SyncCounter_PRODUCER_B;
  uint64 SyncCounter_RING_OUTFEED;
  uint64 avDataPopRunControl;
  uint64 avDataPopBreakPoint;
  uint64 avDataPopRunStatus;
  uint64 avDataPopOverwriteMode;
  uint64 avDataPopEnableTracing;
  uint64 avDataPopStartCycle;
  uint64 avDataPopEndCycle;
  uint64 avDataPopProgramCounter;
  uint64 parameterPopRunControl;
  uint64 parameterPopBreakPoint;
  uint64 parameterPopRunStatus;
  uint64 parameterPopOverwriteMode;
  uint64 parameterPopEnableTracing;
  uint64 parameterPopStartCycle;
  uint64 parameterPopEndCycle;
  uint64 parameterPopProgramCounter;
  uint64 infeedRunControl;
  uint64 infeedRunStatus;
  uint64 infeedBreakPoint;
  uint64 infeedOverwriteMode;
  uint64 infeedEnableTracing;
  uint64 infeedStartCycle;
  uint64 infeedEndCycle;
  uint64 infeedProgramCounter;
  uint64 outfeedRunControl;
  uint64 outfeedRunStatus;
  uint64 outfeedBreakPoint;
  uint64 outfeedOverwriteMode;
  uint64 outfeedEnableTracing;
  uint64 outfeedStartCycle;
  uint64 outfeedEndCycle;
  uint64 outfeedProgramCounter;
  uint64 scalarCoreRunStatus;
  uint64 Error_ScalarCore;
  uint64 Error_Mask_ScalarCore;
  uint64 Error_Force_ScalarCore;
  uint64 Error_Timestamp_ScalarCore;
  uint64 Error_Info_ScalarCore;
  uint64 Timeout;
  uint64 avDataPopTtuStateRegFile;
  uint64 avDataPopTrace;
  uint64 parameterPopTtuStateRegFile;
  uint64 parameterPopTrace;
  uint64 infeedTtuStateRegFile;
  uint64 outfeedTtuStateRegFile;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_SCALAR_CORE_CSR_OFFSETS_H_
