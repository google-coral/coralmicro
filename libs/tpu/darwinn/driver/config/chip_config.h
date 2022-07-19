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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_CHIP_CONFIG_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_CHIP_CONFIG_H_

#include <cassert>

#include "libs/tpu/darwinn/driver/config/aon_reset_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/apex_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/breakpoint_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/cb_bridge_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/chip_structures.h"
#include "libs/tpu/darwinn/driver/config/debug_hib_user_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/debug_scalar_core_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/debug_tile_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/hib_kernel_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/hib_user_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/interrupt_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/memory_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/misc_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/msix_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/queue_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/register_file_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/scalar_core_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/scu_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/sync_flag_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/tile_config_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/tile_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/trace_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/usb_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/wire_csr_offsets.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// Project-independent interface for CSR offsets and system constants.
class ChipConfig {
 public:
  virtual ~ChipConfig() = default;

  // Extracts CSR offsets for various modules in DarwiNN.
  virtual const HibKernelCsrOffsets& GetHibKernelCsrOffsets() const = 0;
  virtual const HibUserCsrOffsets& GetHibUserCsrOffsets() const = 0;
  virtual const QueueCsrOffsets& GetInstructionQueueCsrOffsets() const = 0;
  virtual const ScalarCoreCsrOffsets& GetScalarCoreCsrOffsets() const = 0;
  virtual const TileConfigCsrOffsets& GetTileConfigCsrOffsets() const = 0;
  virtual const TileCsrOffsets& GetTileCsrOffsets() const = 0;
  virtual const InterruptCsrOffsets& GetScalarCoreInterruptCsrOffsets()
      const = 0;
  virtual const InterruptCsrOffsets& GetTopLevelInterruptCsrOffsets() const = 0;
  virtual const InterruptCsrOffsets& GetFatalErrorInterruptCsrOffsets()
      const = 0;

  // Extracts CSR offsets that supports specific functionality in DarwiNN.
  virtual const MsixCsrOffsets& GetMsixCsrOffsets() const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const WireCsrOffsets& GetWireCsrOffsets() const = 0;
  virtual const MiscCsrOffsets& GetMiscCsrOffsets() const {
    assert(false);
    __builtin_unreachable();
  }

  // Extracts chip-specific constants in DarwiNN.
  virtual const ChipStructures& GetChipStructures() const = 0;

  // Extracts CSR offsets used by scalar core debugger in DarwiNN.
  virtual const BreakpointCsrOffsets& GetScalarCoreBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreActivationTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreInfeedTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreOutfeedTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetScalarCoreParameterTtuBreakpointCsrOffsets() const = 0;

  virtual const RegisterFileCsrOffsets& GetScalarRegisterFileCsrOffsets()
      const = 0;
  virtual const RegisterFileCsrOffsets& GetPredicateRegisterFileCsrOffsets()
      const = 0;

  virtual const MemoryCsrOffsets& GetScalarCoreMemoryCsrOffsets() const = 0;

  // Extracts CSR offsets used by tile debugger in DarwiNN.
  virtual const BreakpointCsrOffsets& GetTileOpTtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileWideToNarrowTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileNarrowToWideTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileRingBusConsumer0TtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileRingBusConsumer1TtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets&
  GetTileRingBusProducerTtuBreakpointCsrOffsets() const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus0TtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus1TtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus2TtuBreakpointCsrOffsets()
      const = 0;
  virtual const BreakpointCsrOffsets& GetTileMeshBus3TtuBreakpointCsrOffsets()
      const = 0;

  virtual const MemoryCsrOffsets& GetTileMemoryCsrOffsets() const = 0;

  // Extracts CSR offsets used by scalar core performance tracing.
  virtual const TraceCsrOffsets& GetScalarCoreActivationTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetScalarCoreInfeedTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetScalarCoreOutfeedTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetScalarCoreParameterTtuTraceCsrOffsets()
      const = 0;

  // Extracts CSR offsets used by tile performance tracing.
  virtual const TraceCsrOffsets& GetTileOpTtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileWideToNarrowTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileNarrowToWideTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileRingBusConsumer0TtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileRingBusConsumer1TtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileRingBusProducerTtuTraceCsrOffsets()
      const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus0TtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus1TtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus2TtuTraceCsrOffsets() const = 0;
  virtual const TraceCsrOffsets& GetTileMeshBus3TtuTraceCsrOffsets() const = 0;

  // Extracts CSR offsets used to access sync flags in scalar core.
  virtual const SyncFlagCsrOffsets& GetScalarCoreAvdataPopSyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreParameterPopSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreAvdataInfeedSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreParameterInfeedSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreScalarInfeedSyncFlagCsrOffsets() const = 0;
  virtual const SyncFlagCsrOffsets& GetScalarCoreProducerASyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets& GetScalarCoreProducerBSyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets& GetScalarCoreRingOutfeedSyncFlagCsrOffsets()
      const = 0;
  virtual const SyncFlagCsrOffsets&
  GetScalarCoreScalarPipelineSyncFlagCsrOffsets() const = 0;

  // Extracts CSR offsets used by bug report generator in DarwiNN.
  virtual const DebugHibUserCsrOffsets& GetDebugHibUserCsrOffsets() const = 0;
  virtual const DebugScalarCoreCsrOffsets& GetDebugScalarCoreCsrOffsets()
      const = 0;
  virtual const DebugTileCsrOffsets& GetDebugTileCsrOffsets() const = 0;

  // Beagle-specific.
  virtual const ApexCsrOffsets& GetApexCsrOffsets() const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const ScuCsrOffsets& GetScuCsrOffsets() const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const CbBridgeCsrOffsets& GetCbBridgeCsrOffsets() const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const UsbCsrOffsets& GetUsbCsrOffsets() const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbFatalErrorInterruptCsrOffsets()
      const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel0InterruptCsrOffsets()
      const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel1InterruptCsrOffsets()
      const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel2InterruptCsrOffsets()
      const {
    assert(false);
    __builtin_unreachable();
  }
  virtual const InterruptCsrOffsets& GetUsbTopLevel3InterruptCsrOffsets()
      const {
    assert(false);
    __builtin_unreachable();
  }

  // Jago/Noronha/Abrolhos-specific.
  virtual const AonResetCsrOffsets& GetAonResetCsrOffsets() const {
    assert(false);
    __builtin_unreachable();
  }
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_CHIP_CONFIG_H_
