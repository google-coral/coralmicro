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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_TILE_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_TILE_CSR_OFFSETS_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets that will be dumped as part of the
// driver bug report for tiles. Members are intentionally named to match
// the GCSR register names.
struct DebugTileCsrOffsets {
  uint64_t tileid;
  uint64_t scratchpad;
  uint64_t memoryAccess;
  uint64_t memoryData;
  uint64_t deepSleep;
  uint64_t SyncCounter_AVDATA;
  uint64_t SyncCounter_PARAMETERS;
  uint64_t SyncCounter_PARTIAL_SUMS;
  uint64_t SyncCounter_MESH_NORTH_IN;
  uint64_t SyncCounter_MESH_EAST_IN;
  uint64_t SyncCounter_MESH_SOUTH_IN;
  uint64_t SyncCounter_MESH_WEST_IN;
  uint64_t SyncCounter_MESH_NORTH_OUT;
  uint64_t SyncCounter_MESH_EAST_OUT;
  uint64_t SyncCounter_MESH_SOUTH_OUT;
  uint64_t SyncCounter_MESH_WEST_OUT;
  uint64_t SyncCounter_WIDE_TO_NARROW;
  uint64_t SyncCounter_WIDE_TO_SCALING;
  uint64_t SyncCounter_NARROW_TO_WIDE;
  uint64_t SyncCounter_RING_READ_A;
  uint64_t SyncCounter_RING_READ_B;
  uint64_t SyncCounter_RING_WRITE;
  uint64_t SyncCounter_RING_PRODUCER_A;
  uint64_t SyncCounter_RING_PRODUCER_B;
  uint64_t opRunControl;
  uint64_t PowerSaveData;
  uint64_t opBreakPoint;
  uint64_t StallCounter;
  uint64_t opRunStatus;
  uint64_t OpOverwriteMode;
  uint64_t OpEnableTracing;
  uint64_t OpStartCycle;
  uint64_t OpEndCycle;
  uint64_t OpProgramCounter;
  uint64_t wideToNarrowRunControl;
  uint64_t wideToNarrowRunStatus;
  uint64_t wideToNarrowBreakPoint;
  uint64_t dmaWideToNarrowOverwriteMode;
  uint64_t dmaWideToNarrowEnableTracing;
  uint64_t dmaWideToNarrowStartCycle;
  uint64_t dmaWideToNarrowEndCycle;
  uint64_t dmaWideToNarrowProgramCounter;
  uint64_t narrowToWideRunControl;
  uint64_t narrowToWideRunStatus;
  uint64_t narrowToWideBreakPoint;
  uint64_t dmaNarrowToWideOverwriteMode;
  uint64_t dmaNarrowToWideEnableTracing;
  uint64_t dmaNarrowToWideStartCycle;
  uint64_t dmaNarrowToWideEndCycle;
  uint64_t dmaNarrowToWideProgramCounter;
  uint64_t ringBusConsumer0RunControl;
  uint64_t ringBusConsumer0RunStatus;
  uint64_t ringBusConsumer0BreakPoint;
  uint64_t dmaRingBusConsumer0OverwriteMode;
  uint64_t dmaRingBusConsumer0EnableTracing;
  uint64_t dmaRingBusConsumer0StartCycle;
  uint64_t dmaRingBusConsumer0EndCycle;
  uint64_t dmaRingBusConsumer0ProgramCounter;
  uint64_t ringBusConsumer1RunControl;
  uint64_t ringBusConsumer1RunStatus;
  uint64_t ringBusConsumer1BreakPoint;
  uint64_t dmaRingBusConsumer1OverwriteMode;
  uint64_t dmaRingBusConsumer1EnableTracing;
  uint64_t dmaRingBusConsumer1StartCycle;
  uint64_t dmaRingBusConsumer1EndCycle;
  uint64_t dmaRingBusConsumer1ProgramCounter;
  uint64_t ringBusProducerRunControl;
  uint64_t ringBusProducerRunStatus;
  uint64_t ringBusProducerBreakPoint;
  uint64_t dmaRingBusProducerOverwriteMode;
  uint64_t dmaRingBusProducerEnableTracing;
  uint64_t dmaRingBusProducerStartCycle;
  uint64_t dmaRingBusProducerEndCycle;
  uint64_t dmaRingBusProducerProgramCounter;
  uint64_t meshBus0RunControl;
  uint64_t meshBus0RunStatus;
  uint64_t meshBus0BreakPoint;
  uint64_t dmaMeshBus0OverwriteMode;
  uint64_t dmaMeshBus0EnableTracing;
  uint64_t dmaMeshBus0StartCycle;
  uint64_t dmaMeshBus0EndCycle;
  uint64_t dmaMeshBus0ProgramCounter;
  uint64_t meshBus1RunControl;
  uint64_t meshBus1RunStatus;
  uint64_t meshBus1BreakPoint;
  uint64_t dmaMeshBus1OverwriteMode;
  uint64_t dmaMeshBus1EnableTracing;
  uint64_t dmaMeshBus1StartCycle;
  uint64_t dmaMeshBus1EndCycle;
  uint64_t dmaMeshBus1ProgramCounter;
  uint64_t meshBus2RunControl;
  uint64_t meshBus2RunStatus;
  uint64_t meshBus2BreakPoint;
  uint64_t dmaMeshBus2OverwriteMode;
  uint64_t dmaMeshBus2EnableTracing;
  uint64_t dmaMeshBus2StartCycle;
  uint64_t dmaMeshBus2EndCycle;
  uint64_t dmaMeshBus2ProgramCounter;
  uint64_t meshBus3RunControl;
  uint64_t meshBus3RunStatus;
  uint64_t meshBus3BreakPoint;
  uint64_t dmaMeshBus3OverwriteMode;
  uint64_t dmaMeshBus3EnableTracing;
  uint64_t dmaMeshBus3StartCycle;
  uint64_t dmaMeshBus3EndCycle;
  uint64_t dmaMeshBus3ProgramCounter;
  uint64_t Error_Tile;
  uint64_t Error_Mask_Tile;
  uint64_t Error_Force_Tile;
  uint64_t Error_Timestamp_Tile;
  uint64_t Error_Info_Tile;
  uint64_t Timeout;
  uint64_t opTtuStateRegFile;
  uint64_t OpTrace;
  uint64_t wideToNarrowTtuStateRegFile;
  uint64_t dmaWideToNarrowTrace;
  uint64_t narrowToWideTtuStateRegFile;
  uint64_t dmaNarrowToWideTrace;
  uint64_t ringBusConsumer0TtuStateRegFile;
  uint64_t dmaRingBusConsumer0Trace;
  uint64_t ringBusConsumer1TtuStateRegFile;
  uint64_t dmaRingBusConsumer1Trace;
  uint64_t ringBusProducerTtuStateRegFile;
  uint64_t dmaRingBusProducerTrace;
  uint64_t meshBus0TtuStateRegFile;
  uint64_t dmaMeshBus0Trace;
  uint64_t meshBus1TtuStateRegFile;
  uint64_t dmaMeshBus1Trace;
  uint64_t meshBus2TtuStateRegFile;
  uint64_t dmaMeshBus2Trace;
  uint64_t meshBus3TtuStateRegFile;
  uint64_t dmaMeshBus3Trace;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_TILE_CSR_OFFSETS_H_
