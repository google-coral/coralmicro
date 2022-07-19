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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CSR_OFFSETS_H_

#include "libs/tpu/darwinn/driver/config/apex_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/breakpoint_csr_offsets.h"
#include "libs/tpu/darwinn/driver/config/cb_bridge_csr_offsets.h"
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

const InterruptCsrOffsets kBeagleFatalErrIntInterruptCsrOffsets = {
    0x486c0,  // NOLINT: fatal_err_int_control
    0x486c8,  // NOLINT: fatal_err_int_status
};

const InterruptCsrOffsets kBeagleScHostIntInterruptCsrOffsets = {
    0x486a0,  // NOLINT: sc_host_int_control
    0x486a8,  // NOLINT: sc_host_int_status
};

const InterruptCsrOffsets kBeagleTopLevelIntInterruptCsrOffsets = {
    0x486b0,  // NOLINT: top_level_int_control
    0x486b8,  // NOLINT: top_level_int_status
};

const BreakpointCsrOffsets kBeagleAvdatapopBreakpointCsrOffsets = {
    0x44158,  // NOLINT: avDataPopRunControl
    0x44168,  // NOLINT: avDataPopRunStatus
    0x44160,  // NOLINT: avDataPopBreakPoint
};

const BreakpointCsrOffsets kBeagleInfeedBreakpointCsrOffsets = {
    0x441d8,  // NOLINT: infeedRunControl
    0x441e0,  // NOLINT: infeedRunStatus
    0x441e8,  // NOLINT: infeedBreakPoint
};

const BreakpointCsrOffsets kBeagleOutfeedBreakpointCsrOffsets = {
    0x44218,  // NOLINT: outfeedRunControl
    0x44220,  // NOLINT: outfeedRunStatus
    0x44228,  // NOLINT: outfeedBreakPoint
};

const BreakpointCsrOffsets kBeagleParameterpopBreakpointCsrOffsets = {
    0x44198,  // NOLINT: parameterPopRunControl
    0x441a8,  // NOLINT: parameterPopRunStatus
    0x441a0,  // NOLINT: parameterPopBreakPoint
};

const BreakpointCsrOffsets kBeagleScalarcoreBreakpointCsrOffsets = {
    0x44018,  // NOLINT: scalarCoreRunControl
    0x44258,  // NOLINT: scalarCoreRunStatus
    0x44020,  // NOLINT: scalarCoreBreakPoint
};

const RegisterFileCsrOffsets kBeaglePredicateRegisterFileCsrOffsets = {
    0x44500,  // NOLINT: predicateRegisterFile
};

const RegisterFileCsrOffsets kBeagleScalarRegisterFileCsrOffsets = {
    0x44400,  // NOLINT: scalarRegisterFile
};

const SyncFlagCsrOffsets kBeagleAvdataInfeedSyncFlagCsrOffsets = {
    0x44060,  // NOLINT: SyncCounter_AVDATA_INFEED
};

const SyncFlagCsrOffsets kBeagleAvdataPopSyncFlagCsrOffsets = {
    0x44050,  // NOLINT: SyncCounter_AVDATA_POP
};

const SyncFlagCsrOffsets kBeagleParameterInfeedSyncFlagCsrOffsets = {
    0x44068,  // NOLINT: SyncCounter_PARAMETER_INFEED
};

const SyncFlagCsrOffsets kBeagleParameterPopSyncFlagCsrOffsets = {
    0x44058,  // NOLINT: SyncCounter_PARAMETER_POP
};

const SyncFlagCsrOffsets kBeagleProducerASyncFlagCsrOffsets = {
    0x44078,  // NOLINT: SyncCounter_PRODUCER_A
};

const SyncFlagCsrOffsets kBeagleProducerBSyncFlagCsrOffsets = {
    0x44080,  // NOLINT: SyncCounter_PRODUCER_B
};

const SyncFlagCsrOffsets kBeagleRingOutfeedSyncFlagCsrOffsets = {
    0x44088,  // NOLINT: SyncCounter_RING_OUTFEED
};

const SyncFlagCsrOffsets kBeagleScalarInfeedSyncFlagCsrOffsets = {
    0x44070,  // NOLINT: SyncCounter_SCALAR_INFEED
};

const SyncFlagCsrOffsets kBeagleScalarPipelineSyncFlagCsrOffsets = {
    0x44090,  // NOLINT: SyncCounter_SCALAR_PIPELINE
};

const TraceCsrOffsets kBeagleAvdatapopTraceCsrOffsets = {
    0x44170,  // NOLINT: avDataPopOverwriteMode
    0x44178,  // NOLINT: avDataPopEnableTracing
    0x442c0,  // NOLINT: avDataPopTrace
};

const TraceCsrOffsets kBeagleInfeedTraceCsrOffsets = {
    0x441f0,  // NOLINT: infeedOverwriteMode
    0x441f8,  // NOLINT: infeedEnableTracing
    0x44340,  // NOLINT: infeedTrace
};

const TraceCsrOffsets kBeagleOutfeedTraceCsrOffsets = {
    0x44230,  // NOLINT: outfeedOverwriteMode
    0x44238,  // NOLINT: outfeedEnableTracing
    0x44380,  // NOLINT: outfeedTrace
};

const TraceCsrOffsets kBeagleParameterpopTraceCsrOffsets = {
    0x441b0,  // NOLINT: parameterPopOverwriteMode
    0x441b8,  // NOLINT: parameterPopEnableTracing
    0x44300,  // NOLINT: parameterPopTrace
};

const BreakpointCsrOffsets kBeagleMeshbus0BreakpointCsrOffsets = {
    0x42250,  // NOLINT: meshBus0RunControl
    0x42258,  // NOLINT: meshBus0RunStatus
    0x42260,  // NOLINT: meshBus0BreakPoint
};

const BreakpointCsrOffsets kBeagleMeshbus1BreakpointCsrOffsets = {
    0x42298,  // NOLINT: meshBus1RunControl
    0x422a0,  // NOLINT: meshBus1RunStatus
    0x422a8,  // NOLINT: meshBus1BreakPoint
};

const BreakpointCsrOffsets kBeagleMeshbus2BreakpointCsrOffsets = {
    0x422e0,  // NOLINT: meshBus2RunControl
    0x422e8,  // NOLINT: meshBus2RunStatus
    0x422f0,  // NOLINT: meshBus2BreakPoint
};

const BreakpointCsrOffsets kBeagleMeshbus3BreakpointCsrOffsets = {
    0x42328,  // NOLINT: meshBus3RunControl
    0x42330,  // NOLINT: meshBus3RunStatus
    0x42338,  // NOLINT: meshBus3BreakPoint
};

const BreakpointCsrOffsets kBeagleNarrowtowideBreakpointCsrOffsets = {
    0x42150,  // NOLINT: narrowToWideRunControl
    0x42158,  // NOLINT: narrowToWideRunStatus
    0x42160,  // NOLINT: narrowToWideBreakPoint
};

const BreakpointCsrOffsets kBeagleOpBreakpointCsrOffsets = {
    0x420c0,  // NOLINT: opRunControl
    0x420e0,  // NOLINT: opRunStatus
    0x420d0,  // NOLINT: opBreakPoint
};

const BreakpointCsrOffsets kBeagleRingbusconsumer0BreakpointCsrOffsets = {
    0x42190,  // NOLINT: ringBusConsumer0RunControl
    0x42198,  // NOLINT: ringBusConsumer0RunStatus
    0x421a0,  // NOLINT: ringBusConsumer0BreakPoint
};

const BreakpointCsrOffsets kBeagleRingbusconsumer1BreakpointCsrOffsets = {
    0x421d0,  // NOLINT: ringBusConsumer1RunControl
    0x421d8,  // NOLINT: ringBusConsumer1RunStatus
    0x421e0,  // NOLINT: ringBusConsumer1BreakPoint
};

const BreakpointCsrOffsets kBeagleRingbusproducerBreakpointCsrOffsets = {
    0x42210,  // NOLINT: ringBusProducerRunControl
    0x42218,  // NOLINT: ringBusProducerRunStatus
    0x42220,  // NOLINT: ringBusProducerBreakPoint
};

const BreakpointCsrOffsets kBeagleWidetonarrowBreakpointCsrOffsets = {
    0x42110,  // NOLINT: wideToNarrowRunControl
    0x42118,  // NOLINT: wideToNarrowRunStatus
    0x42120,  // NOLINT: wideToNarrowBreakPoint
};

const SyncFlagCsrOffsets kBeagleAvdataSyncFlagCsrOffsets = {
    0x42028,  // NOLINT: SyncCounter_AVDATA
};

const SyncFlagCsrOffsets kBeagleMeshEastInSyncFlagCsrOffsets = {
    0x42048,  // NOLINT: SyncCounter_MESH_EAST_IN
};

const SyncFlagCsrOffsets kBeagleMeshEastOutSyncFlagCsrOffsets = {
    0x42068,  // NOLINT: SyncCounter_MESH_EAST_OUT
};

const SyncFlagCsrOffsets kBeagleMeshNorthInSyncFlagCsrOffsets = {
    0x42040,  // NOLINT: SyncCounter_MESH_NORTH_IN
};

const SyncFlagCsrOffsets kBeagleMeshNorthOutSyncFlagCsrOffsets = {
    0x42060,  // NOLINT: SyncCounter_MESH_NORTH_OUT
};

const SyncFlagCsrOffsets kBeagleMeshSouthInSyncFlagCsrOffsets = {
    0x42050,  // NOLINT: SyncCounter_MESH_SOUTH_IN
};

const SyncFlagCsrOffsets kBeagleMeshSouthOutSyncFlagCsrOffsets = {
    0x42070,  // NOLINT: SyncCounter_MESH_SOUTH_OUT
};

const SyncFlagCsrOffsets kBeagleMeshWestInSyncFlagCsrOffsets = {
    0x42058,  // NOLINT: SyncCounter_MESH_WEST_IN
};

const SyncFlagCsrOffsets kBeagleMeshWestOutSyncFlagCsrOffsets = {
    0x42078,  // NOLINT: SyncCounter_MESH_WEST_OUT
};

const SyncFlagCsrOffsets kBeagleNarrowToWideSyncFlagCsrOffsets = {
    0x42090,  // NOLINT: SyncCounter_NARROW_TO_WIDE
};

const SyncFlagCsrOffsets kBeagleParametersSyncFlagCsrOffsets = {
    0x42030,  // NOLINT: SyncCounter_PARAMETERS
};

const SyncFlagCsrOffsets kBeaglePartialSumsSyncFlagCsrOffsets = {
    0x42038,  // NOLINT: SyncCounter_PARTIAL_SUMS
};

const SyncFlagCsrOffsets kBeagleRingProducerASyncFlagCsrOffsets = {
    0x420b0,  // NOLINT: SyncCounter_RING_PRODUCER_A
};

const SyncFlagCsrOffsets kBeagleRingProducerBSyncFlagCsrOffsets = {
    0x420b8,  // NOLINT: SyncCounter_RING_PRODUCER_B
};

const SyncFlagCsrOffsets kBeagleRingReadASyncFlagCsrOffsets = {
    0x42098,  // NOLINT: SyncCounter_RING_READ_A
};

const SyncFlagCsrOffsets kBeagleRingReadBSyncFlagCsrOffsets = {
    0x420a0,  // NOLINT: SyncCounter_RING_READ_B
};

const SyncFlagCsrOffsets kBeagleRingWriteSyncFlagCsrOffsets = {
    0x420a8,  // NOLINT: SyncCounter_RING_WRITE
};

const SyncFlagCsrOffsets kBeagleWideToNarrowSyncFlagCsrOffsets = {
    0x42080,  // NOLINT: SyncCounter_WIDE_TO_NARROW
};

const SyncFlagCsrOffsets kBeagleWideToScalingSyncFlagCsrOffsets = {
    0x42088,  // NOLINT: SyncCounter_WIDE_TO_SCALING
};

const TraceCsrOffsets kBeagleDmameshbus0TraceCsrOffsets = {
    0x42270,  // NOLINT: dmaMeshBus0OverwriteMode
    0x42278,  // NOLINT: dmaMeshBus0EnableTracing
    0x42740,  // NOLINT: dmaMeshBus0Trace
};

const TraceCsrOffsets kBeagleDmameshbus1TraceCsrOffsets = {
    0x422b8,  // NOLINT: dmaMeshBus1OverwriteMode
    0x422c0,  // NOLINT: dmaMeshBus1EnableTracing
    0x427c0,  // NOLINT: dmaMeshBus1Trace
};

const TraceCsrOffsets kBeagleDmameshbus2TraceCsrOffsets = {
    0x42300,  // NOLINT: dmaMeshBus2OverwriteMode
    0x42308,  // NOLINT: dmaMeshBus2EnableTracing
    0x42840,  // NOLINT: dmaMeshBus2Trace
};

const TraceCsrOffsets kBeagleDmameshbus3TraceCsrOffsets = {
    0x42348,  // NOLINT: dmaMeshBus3OverwriteMode
    0x42350,  // NOLINT: dmaMeshBus3EnableTracing
    0x428c0,  // NOLINT: dmaMeshBus3Trace
};

const TraceCsrOffsets kBeagleDmanarrowtowideTraceCsrOffsets = {
    0x42168,  // NOLINT: dmaNarrowToWideOverwriteMode
    0x42170,  // NOLINT: dmaNarrowToWideEnableTracing
    0x42600,  // NOLINT: dmaNarrowToWideTrace
};

const TraceCsrOffsets kBeagleDmaringbusconsumer0TraceCsrOffsets = {
    0x421a8,  // NOLINT: dmaRingBusConsumer0OverwriteMode
    0x421b0,  // NOLINT: dmaRingBusConsumer0EnableTracing
    0x42640,  // NOLINT: dmaRingBusConsumer0Trace
};

const TraceCsrOffsets kBeagleDmaringbusconsumer1TraceCsrOffsets = {
    0x421e8,  // NOLINT: dmaRingBusConsumer1OverwriteMode
    0x421f0,  // NOLINT: dmaRingBusConsumer1EnableTracing
    0x42680,  // NOLINT: dmaRingBusConsumer1Trace
};

const TraceCsrOffsets kBeagleDmaringbusproducerTraceCsrOffsets = {
    0x42228,  // NOLINT: dmaRingBusProducerOverwriteMode
    0x42230,  // NOLINT: dmaRingBusProducerEnableTracing
    0x426c0,  // NOLINT: dmaRingBusProducerTrace
};

const TraceCsrOffsets kBeagleDmawidetonarrowTraceCsrOffsets = {
    0x42128,  // NOLINT: dmaWideToNarrowOverwriteMode
    0x42130,  // NOLINT: dmaWideToNarrowEnableTracing
    0x42500,  // NOLINT: dmaWideToNarrowTrace
};

const TraceCsrOffsets kBeagleOpTraceCsrOffsets = {
    0x420e8,  // NOLINT: OpOverwriteMode
    0x420f0,  // NOLINT: OpEnableTracing
    0x42400,  // NOLINT: OpTrace
};

const DebugHibUserCsrOffsets kBeagleDebugHibUserCsrOffsets = {
    0x48010,  // NOLINT: instruction_inbound_queue_total_occupancy
    0x48018,  // NOLINT: instruction_inbound_queue_threshold_counter
    0x48020,  // NOLINT: instruction_inbound_queue_insertion_counter
    0x48028,  // NOLINT: instruction_inbound_queue_full_counter
    0x48030,  // NOLINT: input_actv_inbound_queue_total_occupancy
    0x48038,  // NOLINT: input_actv_inbound_queue_threshold_counter
    0x48040,  // NOLINT: input_actv_inbound_queue_insertion_counter
    0x48048,  // NOLINT: input_actv_inbound_queue_full_counter
    0x48050,  // NOLINT: param_inbound_queue_total_occupancy
    0x48058,  // NOLINT: param_inbound_queue_threshold_counter
    0x48060,  // NOLINT: param_inbound_queue_insertion_counter
    0x48068,  // NOLINT: param_inbound_queue_full_counter
    0x48070,  // NOLINT: output_actv_inbound_queue_total_occupancy
    0x48078,  // NOLINT: output_actv_inbound_queue_threshold_counter
    0x48080,  // NOLINT: output_actv_inbound_queue_insertion_counter
    0x48088,  // NOLINT: output_actv_inbound_queue_full_counter
    0x48090,  // NOLINT: status_block_write_inbound_queue_total_occupancy
    0x48098,  // NOLINT: status_block_write_inbound_queue_threshold_counter
    0x480a0,  // NOLINT: status_block_write_inbound_queue_insertion_counter
    0x480a8,  // NOLINT: status_block_write_inbound_queue_full_counter
    0x480b0,  // NOLINT: queue_fetch_inbound_queue_total_occupancy
    0x480b8,  // NOLINT: queue_fetch_inbound_queue_threshold_counter
    0x480c0,  // NOLINT: queue_fetch_inbound_queue_insertion_counter
    0x480c8,  // NOLINT: queue_fetch_inbound_queue_full_counter
    0x480d0,  // NOLINT: instruction_outbound_queue_total_occupancy
    0x480d8,  // NOLINT: instruction_outbound_queue_threshold_counter
    0x480e0,  // NOLINT: instruction_outbound_queue_insertion_counter
    0x480e8,  // NOLINT: instruction_outbound_queue_full_counter
    0x480f0,  // NOLINT: input_actv_outbound_queue_total_occupancy
    0x480f8,  // NOLINT: input_actv_outbound_queue_threshold_counter
    0x48100,  // NOLINT: input_actv_outbound_queue_insertion_counter
    0x48108,  // NOLINT: input_actv_outbound_queue_full_counter
    0x48110,  // NOLINT: param_outbound_queue_total_occupancy
    0x48118,  // NOLINT: param_outbound_queue_threshold_counter
    0x48120,  // NOLINT: param_outbound_queue_insertion_counter
    0x48128,  // NOLINT: param_outbound_queue_full_counter
    0x48130,  // NOLINT: output_actv_outbound_queue_total_occupancy
    0x48138,  // NOLINT: output_actv_outbound_queue_threshold_counter
    0x48140,  // NOLINT: output_actv_outbound_queue_insertion_counter
    0x48148,  // NOLINT: output_actv_outbound_queue_full_counter
    0x48150,  // NOLINT: status_block_write_outbound_queue_total_occupancy
    0x48158,  // NOLINT: status_block_write_outbound_queue_threshold_counter
    0x48160,  // NOLINT: status_block_write_outbound_queue_insertion_counter
    0x48168,  // NOLINT: status_block_write_outbound_queue_full_counter
    0x48170,  // NOLINT: queue_fetch_outbound_queue_total_occupancy
    0x48178,  // NOLINT: queue_fetch_outbound_queue_threshold_counter
    0x48180,  // NOLINT: queue_fetch_outbound_queue_insertion_counter
    0x48188,  // NOLINT: queue_fetch_outbound_queue_full_counter
    0x48190,  // NOLINT: page_table_request_outbound_queue_total_occupancy
    0x48198,  // NOLINT: page_table_request_outbound_queue_threshold_counter
    0x481a0,  // NOLINT: page_table_request_outbound_queue_insertion_counter
    0x481a8,  // NOLINT: page_table_request_outbound_queue_full_counter
    0x481b0,  // NOLINT: read_tracking_fifo_total_occupancy
    0x481b8,  // NOLINT: read_tracking_fifo_threshold_counter
    0x481c0,  // NOLINT: read_tracking_fifo_insertion_counter
    0x481c8,  // NOLINT: read_tracking_fifo_full_counter
    0x481d0,  // NOLINT: write_tracking_fifo_total_occupancy
    0x481d8,  // NOLINT: write_tracking_fifo_threshold_counter
    0x481e0,  // NOLINT: write_tracking_fifo_insertion_counter
    0x481e8,  // NOLINT: write_tracking_fifo_full_counter
    0x481f0,  // NOLINT: read_buffer_total_occupancy
    0x481f8,  // NOLINT: read_buffer_threshold_counter
    0x48200,  // NOLINT: read_buffer_insertion_counter
    0x48208,  // NOLINT: read_buffer_full_counter
    0x48210,  // NOLINT: axi_aw_credit_shim_total_occupancy
    0x48218,  // NOLINT: axi_aw_credit_shim_threshold_counter
    0x48220,  // NOLINT: axi_aw_credit_shim_insertion_counter
    0x48228,  // NOLINT: axi_aw_credit_shim_full_counter
    0x48230,  // NOLINT: axi_ar_credit_shim_total_occupancy
    0x48238,  // NOLINT: axi_ar_credit_shim_threshold_counter
    0x48240,  // NOLINT: axi_ar_credit_shim_insertion_counter
    0x48248,  // NOLINT: axi_ar_credit_shim_full_counter
    0x48250,  // NOLINT: axi_w_credit_shim_total_occupancy
    0x48258,  // NOLINT: axi_w_credit_shim_threshold_counter
    0x48260,  // NOLINT: axi_w_credit_shim_insertion_counter
    0x48268,  // NOLINT: axi_w_credit_shim_full_counter
    0x48270,  // NOLINT: instruction_inbound_queue_empty_cycles_count
    0x48278,  // NOLINT: input_actv_inbound_queue_empty_cycles_count
    0x48280,  // NOLINT: param_inbound_queue_empty_cycles_count
    0x48288,  // NOLINT: output_actv_inbound_queue_empty_cycles_count
    0x48290,  // NOLINT: status_block_write_inbound_queue_empty_cycles_count
    0x48298,  // NOLINT: queue_fetch_inbound_queue_empty_cycles_count
    0x482a0,  // NOLINT: instruction_outbound_queue_empty_cycles_count
    0x482a8,  // NOLINT: input_actv_outbound_queue_empty_cycles_count
    0x482b0,  // NOLINT: param_outbound_queue_empty_cycles_count
    0x482b8,  // NOLINT: output_actv_outbound_queue_empty_cycles_count
    0x482c0,  // NOLINT: status_block_write_outbound_queue_empty_cycles_count
    0x482c8,  // NOLINT: queue_fetch_outbound_queue_empty_cycles_count
    0x482d0,  // NOLINT: page_table_request_outbound_queue_empty_cycles_count
    0x482d8,  // NOLINT: read_tracking_fifo_empty_cycles_count
    0x482e0,  // NOLINT: write_tracking_fifo_empty_cycles_count
    0x482e8,  // NOLINT: read_buffer_empty_cycles_count
    0x482f0,  // NOLINT: read_request_arbiter_instruction_request_cycles
    0x482f8,  // NOLINT: read_request_arbiter_instruction_blocked_cycles
    0x48300,  // NOLINT:
              // read_request_arbiter_instruction_blocked_by_arbitration_cycles
    0x48308,  // NOLINT:
              // read_request_arbiter_instruction_cycles_blocked_over_threshold
    0x48310,  // NOLINT: read_request_arbiter_input_actv_request_cycles
    0x48318,  // NOLINT: read_request_arbiter_input_actv_blocked_cycles
    0x48320,  // NOLINT:
              // read_request_arbiter_input_actv_blocked_by_arbitration_cycles
    0x48328,  // NOLINT:
              // read_request_arbiter_input_actv_cycles_blocked_over_threshold
    0x48330,  // NOLINT: read_request_arbiter_param_request_cycles
    0x48338,  // NOLINT: read_request_arbiter_param_blocked_cycles
    0x48340,  // NOLINT:
              // read_request_arbiter_param_blocked_by_arbitration_cycles
    0x48348,  // NOLINT:
              // read_request_arbiter_param_cycles_blocked_over_threshold
    0x48350,  // NOLINT: read_request_arbiter_queue_fetch_request_cycles
    0x48358,  // NOLINT: read_request_arbiter_queue_fetch_blocked_cycles
    0x48360,  // NOLINT:
              // read_request_arbiter_queue_fetch_blocked_by_arbitration_cycles
    0x48368,  // NOLINT:
              // read_request_arbiter_queue_fetch_cycles_blocked_over_threshold
    0x48370,  // NOLINT: read_request_arbiter_page_table_request_request_cycles
    0x48378,  // NOLINT: read_request_arbiter_page_table_request_blocked_cycles
    0x48380,  // NOLINT:
              // read_request_arbiter_page_table_request_blocked_by_arbitration_cycles
    0x48388,  // NOLINT:
              // read_request_arbiter_page_table_request_cycles_blocked_over_threshold
    0x48390,  // NOLINT: write_request_arbiter_output_actv_request_cycles
    0x48398,  // NOLINT: write_request_arbiter_output_actv_blocked_cycles
    0x483a0,  // NOLINT:
              // write_request_arbiter_output_actv_blocked_by_arbitration_cycles
    0x483a8,  // NOLINT:
              // write_request_arbiter_output_actv_cycles_blocked_over_threshold
    0x483b0,  // NOLINT: write_request_arbiter_status_block_write_request_cycles
    0x483b8,  // NOLINT: write_request_arbiter_status_block_write_blocked_cycles
    0x483c0,  // NOLINT:
              // write_request_arbiter_status_block_write_blocked_by_arbitration_cycles
    0x483c8,  // NOLINT:
              // write_request_arbiter_status_block_write_cycles_blocked_over_threshold
    0x483d0,  // NOLINT: address_translation_arbiter_instruction_request_cycles
    0x483d8,  // NOLINT: address_translation_arbiter_instruction_blocked_cycles
    0x483e0,  // NOLINT:
              // address_translation_arbiter_instruction_blocked_by_arbitration_cycles
    0x483e8,  // NOLINT:
              // address_translation_arbiter_instruction_cycles_blocked_over_threshold
    0x483f0,  // NOLINT: address_translation_arbiter_input_actv_request_cycles
    0x483f8,  // NOLINT: address_translation_arbiter_input_actv_blocked_cycles
    0x48400,  // NOLINT:
              // address_translation_arbiter_input_actv_blocked_by_arbitration_cycles
    0x48408,  // NOLINT:
              // address_translation_arbiter_input_actv_cycles_blocked_over_threshold
    0x48410,  // NOLINT: address_translation_arbiter_param_request_cycles
    0x48418,  // NOLINT: address_translation_arbiter_param_blocked_cycles
    0x48420,  // NOLINT:
              // address_translation_arbiter_param_blocked_by_arbitration_cycles
    0x48428,  // NOLINT:
              // address_translation_arbiter_param_cycles_blocked_over_threshold
    0x48430,  // NOLINT:
              // address_translation_arbiter_status_block_write_request_cycles
    0x48438,  // NOLINT:
              // address_translation_arbiter_status_block_write_blocked_cycles
    0x48440,  // NOLINT:
              // address_translation_arbiter_status_block_write_blocked_by_arbitration_cycles
    0x48448,  // NOLINT:
              // address_translation_arbiter_status_block_write_cycles_blocked_over_threshold
    0x48450,  // NOLINT: address_translation_arbiter_output_actv_request_cycles
    0x48458,  // NOLINT: address_translation_arbiter_output_actv_blocked_cycles
    0x48460,  // NOLINT:
              // address_translation_arbiter_output_actv_blocked_by_arbitration_cycles
    0x48468,  // NOLINT:
              // address_translation_arbiter_output_actv_cycles_blocked_over_threshold
    0x48470,  // NOLINT: address_translation_arbiter_queue_fetch_request_cycles
    0x48478,  // NOLINT: address_translation_arbiter_queue_fetch_blocked_cycles
    0x48480,  // NOLINT:
              // address_translation_arbiter_queue_fetch_blocked_by_arbitration_cycles
    0x48488,  // NOLINT:
              // address_translation_arbiter_queue_fetch_cycles_blocked_over_threshold
    0x48490,  // NOLINT: issued_interrupt_count
    0x48498,  // NOLINT: data_read_16byte_count
    0x484a0,  // NOLINT: waiting_for_tag_cycles
    0x484a8,  // NOLINT: waiting_for_axi_cycles
    0x484b0,  // NOLINT: simple_translations
    0x484c8,  // NOLINT: instruction_credits_per_cycle_sum
    0x484d0,  // NOLINT: input_actv_credits_per_cycle_sum
    0x484d8,  // NOLINT: param_credits_per_cycle_sum
    0x484e0,  // NOLINT: output_actv_credits_per_cycle_sum
    0x484e8,  // NOLINT: status_block_write_credits_per_cycle_sum
    0x484f0,  // NOLINT: queue_fetch_credits_per_cycle_sum
    0x484f8,  // NOLINT: page_table_request_credits_per_cycle_sum
    0x48500,  // NOLINT: output_actv_queue_control
    0x48508,  // NOLINT: output_actv_queue_status
    0x48510,  // NOLINT: output_actv_queue_descriptor_size
    0x48518,  // NOLINT: output_actv_queue_minimum_size
    0x48520,  // NOLINT: output_actv_queue_maximum_size
    0x48528,  // NOLINT: output_actv_queue_base
    0x48530,  // NOLINT: output_actv_queue_status_block_base
    0x48538,  // NOLINT: output_actv_queue_size
    0x48540,  // NOLINT: output_actv_queue_tail
    0x48548,  // NOLINT: output_actv_queue_fetched_head
    0x48550,  // NOLINT: output_actv_queue_completed_head
    0x48558,  // NOLINT: output_actv_queue_int_control
    0x48560,  // NOLINT: output_actv_queue_int_status
    0x48568,  // NOLINT: instruction_queue_control
    0x48570,  // NOLINT: instruction_queue_status
    0x48578,  // NOLINT: instruction_queue_descriptor_size
    0x48580,  // NOLINT: instruction_queue_minimum_size
    0x48588,  // NOLINT: instruction_queue_maximum_size
    0x48590,  // NOLINT: instruction_queue_base
    0x48598,  // NOLINT: instruction_queue_status_block_base
    0x485a0,  // NOLINT: instruction_queue_size
    0x485a8,  // NOLINT: instruction_queue_tail
    0x485b0,  // NOLINT: instruction_queue_fetched_head
    0x485b8,  // NOLINT: instruction_queue_completed_head
    0x485c0,  // NOLINT: instruction_queue_int_control
    0x485c8,  // NOLINT: instruction_queue_int_status
    0x485d0,  // NOLINT: input_actv_queue_control
    0x485d8,  // NOLINT: input_actv_queue_status
    0x485e0,  // NOLINT: input_actv_queue_descriptor_size
    0x485e8,  // NOLINT: input_actv_queue_minimum_size
    0x485f0,  // NOLINT: input_actv_queue_maximum_size
    0x485f8,  // NOLINT: input_actv_queue_base
    0x48600,  // NOLINT: input_actv_queue_status_block_base
    0x48608,  // NOLINT: input_actv_queue_size
    0x48610,  // NOLINT: input_actv_queue_tail
    0x48618,  // NOLINT: input_actv_queue_fetched_head
    0x48620,  // NOLINT: input_actv_queue_completed_head
    0x48628,  // NOLINT: input_actv_queue_int_control
    0x48630,  // NOLINT: input_actv_queue_int_status
    0x48638,  // NOLINT: param_queue_control
    0x48640,  // NOLINT: param_queue_status
    0x48648,  // NOLINT: param_queue_descriptor_size
    0x48650,  // NOLINT: param_queue_minimum_size
    0x48658,  // NOLINT: param_queue_maximum_size
    0x48660,  // NOLINT: param_queue_base
    0x48668,  // NOLINT: param_queue_status_block_base
    0x48670,  // NOLINT: param_queue_size
    0x48678,  // NOLINT: param_queue_tail
    0x48680,  // NOLINT: param_queue_fetched_head
    0x48688,  // NOLINT: param_queue_completed_head
    0x48690,  // NOLINT: param_queue_int_control
    0x48698,  // NOLINT: param_queue_int_status
    0x486a0,  // NOLINT: sc_host_int_control
    0x486a8,  // NOLINT: sc_host_int_status
    0x486b0,  // NOLINT: top_level_int_control
    0x486b8,  // NOLINT: top_level_int_status
    0x486c0,  // NOLINT: fatal_err_int_control
    0x486c8,  // NOLINT: fatal_err_int_status
    0x486d0,  // NOLINT: sc_host_int_count
    0x486d8,  // NOLINT: dma_pause
    0x486e0,  // NOLINT: dma_paused
    0x486e8,  // NOLINT: status_block_update
    0x486f0,  // NOLINT: hib_error_status
    0x486f8,  // NOLINT: hib_error_mask
    0x48700,  // NOLINT: hib_first_error_status
    0x48708,  // NOLINT: hib_first_error_timestamp
    0x48710,  // NOLINT: hib_inject_error
    0x48718,  // NOLINT: read_request_arbiter
    0x48720,  // NOLINT: write_request_arbiter
    0x48728,  // NOLINT: address_translation_arbiter
    0x48730,  // NOLINT: sender_queue_threshold
    0x48738,  // NOLINT: page_fault_address
    0x48740,  // NOLINT: instruction_credits
    0x48748,  // NOLINT: input_actv_credits
    0x48750,  // NOLINT: param_credits
    0x48758,  // NOLINT: output_actv_credits
    0x48760,  // NOLINT: pause_state
    0x48768,  // NOLINT: snapshot
    0x48770,  // NOLINT: idle_assert
    0x48778,  // NOLINT: wire_int_pending_bit_array
    0x48788,  // NOLINT: tileconfig0
    0x48790,  // NOLINT: tileconfig1
};

const DebugScalarCoreCsrOffsets kBeagleDebugScalarCoreCsrOffsets = {
    0x44000,  // NOLINT: topology
    0x44008,  // NOLINT: scMemoryCapacity
    0x44010,  // NOLINT: tileMemoryCapacity
    0x44018,  // NOLINT: scalarCoreRunControl
    0x44020,  // NOLINT: scalarCoreBreakPoint
    0x44028,  // NOLINT: currentPc
    0x44038,  // NOLINT: executeControl
    0x44040,  // NOLINT: scMemoryAccess
    0x44048,  // NOLINT: scMemoryData
    0x44050,  // NOLINT: SyncCounter_AVDATA_POP
    0x44058,  // NOLINT: SyncCounter_PARAMETER_POP
    0x44060,  // NOLINT: SyncCounter_AVDATA_INFEED
    0x44068,  // NOLINT: SyncCounter_PARAMETER_INFEED
    0x44070,  // NOLINT: SyncCounter_SCALAR_INFEED
    0x44078,  // NOLINT: SyncCounter_PRODUCER_A
    0x44080,  // NOLINT: SyncCounter_PRODUCER_B
    0x44088,  // NOLINT: SyncCounter_RING_OUTFEED
    0x44158,  // NOLINT: avDataPopRunControl
    0x44160,  // NOLINT: avDataPopBreakPoint
    0x44168,  // NOLINT: avDataPopRunStatus
    0x44170,  // NOLINT: avDataPopOverwriteMode
    0x44178,  // NOLINT: avDataPopEnableTracing
    0x44180,  // NOLINT: avDataPopStartCycle
    0x44188,  // NOLINT: avDataPopEndCycle
    0x44190,  // NOLINT: avDataPopProgramCounter
    0x44198,  // NOLINT: parameterPopRunControl
    0x441a0,  // NOLINT: parameterPopBreakPoint
    0x441a8,  // NOLINT: parameterPopRunStatus
    0x441b0,  // NOLINT: parameterPopOverwriteMode
    0x441b8,  // NOLINT: parameterPopEnableTracing
    0x441c0,  // NOLINT: parameterPopStartCycle
    0x441c8,  // NOLINT: parameterPopEndCycle
    0x441d0,  // NOLINT: parameterPopProgramCounter
    0x441d8,  // NOLINT: infeedRunControl
    0x441e0,  // NOLINT: infeedRunStatus
    0x441e8,  // NOLINT: infeedBreakPoint
    0x441f0,  // NOLINT: infeedOverwriteMode
    0x441f8,  // NOLINT: infeedEnableTracing
    0x44200,  // NOLINT: infeedStartCycle
    0x44208,  // NOLINT: infeedEndCycle
    0x44210,  // NOLINT: infeedProgramCounter
    0x44218,  // NOLINT: outfeedRunControl
    0x44220,  // NOLINT: outfeedRunStatus
    0x44228,  // NOLINT: outfeedBreakPoint
    0x44230,  // NOLINT: outfeedOverwriteMode
    0x44238,  // NOLINT: outfeedEnableTracing
    0x44240,  // NOLINT: outfeedStartCycle
    0x44248,  // NOLINT: outfeedEndCycle
    0x44250,  // NOLINT: outfeedProgramCounter
    0x44258,  // NOLINT: scalarCoreRunStatus
    0x44260,  // NOLINT: Error_ScalarCore
    0x44268,  // NOLINT: Error_Mask_ScalarCore
    0x44270,  // NOLINT: Error_Force_ScalarCore
    0x44278,  // NOLINT: Error_Timestamp_ScalarCore
    0x44280,  // NOLINT: Error_Info_ScalarCore
    0x44288,  // NOLINT: Timeout
    0x442a0,  // NOLINT: avDataPopTtuStateRegFile
    0x442c0,  // NOLINT: avDataPopTrace
    0x442e0,  // NOLINT: parameterPopTtuStateRegFile
    0x44300,  // NOLINT: parameterPopTrace
    0x44320,  // NOLINT: infeedTtuStateRegFile
    0x44360,  // NOLINT: outfeedTtuStateRegFile
};

const DebugTileCsrOffsets kBeagleDebugTileCsrOffsets = {
    0x42000,  // NOLINT: tileid
    0x42008,  // NOLINT: scratchpad
    0x42010,  // NOLINT: memoryAccess
    0x42018,  // NOLINT: memoryData
    0x42020,  // NOLINT: deepSleep
    0x42028,  // NOLINT: SyncCounter_AVDATA
    0x42030,  // NOLINT: SyncCounter_PARAMETERS
    0x42038,  // NOLINT: SyncCounter_PARTIAL_SUMS
    0x42040,  // NOLINT: SyncCounter_MESH_NORTH_IN
    0x42048,  // NOLINT: SyncCounter_MESH_EAST_IN
    0x42050,  // NOLINT: SyncCounter_MESH_SOUTH_IN
    0x42058,  // NOLINT: SyncCounter_MESH_WEST_IN
    0x42060,  // NOLINT: SyncCounter_MESH_NORTH_OUT
    0x42068,  // NOLINT: SyncCounter_MESH_EAST_OUT
    0x42070,  // NOLINT: SyncCounter_MESH_SOUTH_OUT
    0x42078,  // NOLINT: SyncCounter_MESH_WEST_OUT
    0x42080,  // NOLINT: SyncCounter_WIDE_TO_NARROW
    0x42088,  // NOLINT: SyncCounter_WIDE_TO_SCALING
    0x42090,  // NOLINT: SyncCounter_NARROW_TO_WIDE
    0x42098,  // NOLINT: SyncCounter_RING_READ_A
    0x420a0,  // NOLINT: SyncCounter_RING_READ_B
    0x420a8,  // NOLINT: SyncCounter_RING_WRITE
    0x420b0,  // NOLINT: SyncCounter_RING_PRODUCER_A
    0x420b8,  // NOLINT: SyncCounter_RING_PRODUCER_B
    0x420c0,  // NOLINT: opRunControl
    0x420c8,  // NOLINT: PowerSaveData
    0x420d0,  // NOLINT: opBreakPoint
    0x420d8,  // NOLINT: StallCounter
    0x420e0,  // NOLINT: opRunStatus
    0x420e8,  // NOLINT: OpOverwriteMode
    0x420f0,  // NOLINT: OpEnableTracing
    0x420f8,  // NOLINT: OpStartCycle
    0x42100,  // NOLINT: OpEndCycle
    0x42108,  // NOLINT: OpProgramCounter
    0x42110,  // NOLINT: wideToNarrowRunControl
    0x42118,  // NOLINT: wideToNarrowRunStatus
    0x42120,  // NOLINT: wideToNarrowBreakPoint
    0x42128,  // NOLINT: dmaWideToNarrowOverwriteMode
    0x42130,  // NOLINT: dmaWideToNarrowEnableTracing
    0x42138,  // NOLINT: dmaWideToNarrowStartCycle
    0x42140,  // NOLINT: dmaWideToNarrowEndCycle
    0x42148,  // NOLINT: dmaWideToNarrowProgramCounter
    0x42150,  // NOLINT: narrowToWideRunControl
    0x42158,  // NOLINT: narrowToWideRunStatus
    0x42160,  // NOLINT: narrowToWideBreakPoint
    0x42168,  // NOLINT: dmaNarrowToWideOverwriteMode
    0x42170,  // NOLINT: dmaNarrowToWideEnableTracing
    0x42178,  // NOLINT: dmaNarrowToWideStartCycle
    0x42180,  // NOLINT: dmaNarrowToWideEndCycle
    0x42188,  // NOLINT: dmaNarrowToWideProgramCounter
    0x42190,  // NOLINT: ringBusConsumer0RunControl
    0x42198,  // NOLINT: ringBusConsumer0RunStatus
    0x421a0,  // NOLINT: ringBusConsumer0BreakPoint
    0x421a8,  // NOLINT: dmaRingBusConsumer0OverwriteMode
    0x421b0,  // NOLINT: dmaRingBusConsumer0EnableTracing
    0x421b8,  // NOLINT: dmaRingBusConsumer0StartCycle
    0x421c0,  // NOLINT: dmaRingBusConsumer0EndCycle
    0x421c8,  // NOLINT: dmaRingBusConsumer0ProgramCounter
    0x421d0,  // NOLINT: ringBusConsumer1RunControl
    0x421d8,  // NOLINT: ringBusConsumer1RunStatus
    0x421e0,  // NOLINT: ringBusConsumer1BreakPoint
    0x421e8,  // NOLINT: dmaRingBusConsumer1OverwriteMode
    0x421f0,  // NOLINT: dmaRingBusConsumer1EnableTracing
    0x421f8,  // NOLINT: dmaRingBusConsumer1StartCycle
    0x42200,  // NOLINT: dmaRingBusConsumer1EndCycle
    0x42208,  // NOLINT: dmaRingBusConsumer1ProgramCounter
    0x42210,  // NOLINT: ringBusProducerRunControl
    0x42218,  // NOLINT: ringBusProducerRunStatus
    0x42220,  // NOLINT: ringBusProducerBreakPoint
    0x42228,  // NOLINT: dmaRingBusProducerOverwriteMode
    0x42230,  // NOLINT: dmaRingBusProducerEnableTracing
    0x42238,  // NOLINT: dmaRingBusProducerStartCycle
    0x42240,  // NOLINT: dmaRingBusProducerEndCycle
    0x42248,  // NOLINT: dmaRingBusProducerProgramCounter
    0x42250,  // NOLINT: meshBus0RunControl
    0x42258,  // NOLINT: meshBus0RunStatus
    0x42260,  // NOLINT: meshBus0BreakPoint
    0x42270,  // NOLINT: dmaMeshBus0OverwriteMode
    0x42278,  // NOLINT: dmaMeshBus0EnableTracing
    0x42280,  // NOLINT: dmaMeshBus0StartCycle
    0x42288,  // NOLINT: dmaMeshBus0EndCycle
    0x42290,  // NOLINT: dmaMeshBus0ProgramCounter
    0x42298,  // NOLINT: meshBus1RunControl
    0x422a0,  // NOLINT: meshBus1RunStatus
    0x422a8,  // NOLINT: meshBus1BreakPoint
    0x422b8,  // NOLINT: dmaMeshBus1OverwriteMode
    0x422c0,  // NOLINT: dmaMeshBus1EnableTracing
    0x422c8,  // NOLINT: dmaMeshBus1StartCycle
    0x422d0,  // NOLINT: dmaMeshBus1EndCycle
    0x422d8,  // NOLINT: dmaMeshBus1ProgramCounter
    0x422e0,  // NOLINT: meshBus2RunControl
    0x422e8,  // NOLINT: meshBus2RunStatus
    0x422f0,  // NOLINT: meshBus2BreakPoint
    0x42300,  // NOLINT: dmaMeshBus2OverwriteMode
    0x42308,  // NOLINT: dmaMeshBus2EnableTracing
    0x42310,  // NOLINT: dmaMeshBus2StartCycle
    0x42318,  // NOLINT: dmaMeshBus2EndCycle
    0x42320,  // NOLINT: dmaMeshBus2ProgramCounter
    0x42328,  // NOLINT: meshBus3RunControl
    0x42330,  // NOLINT: meshBus3RunStatus
    0x42338,  // NOLINT: meshBus3BreakPoint
    0x42348,  // NOLINT: dmaMeshBus3OverwriteMode
    0x42350,  // NOLINT: dmaMeshBus3EnableTracing
    0x42358,  // NOLINT: dmaMeshBus3StartCycle
    0x42360,  // NOLINT: dmaMeshBus3EndCycle
    0x42368,  // NOLINT: dmaMeshBus3ProgramCounter
    0x42370,  // NOLINT: Error_Tile
    0x42378,  // NOLINT: Error_Mask_Tile
    0x42380,  // NOLINT: Error_Force_Tile
    0x42388,  // NOLINT: Error_Timestamp_Tile
    0x42390,  // NOLINT: Error_Info_Tile
    0x42398,  // NOLINT: Timeout
    0x423c0,  // NOLINT: opTtuStateRegFile
    0x42400,  // NOLINT: OpTrace
    0x42480,  // NOLINT: wideToNarrowTtuStateRegFile
    0x42500,  // NOLINT: dmaWideToNarrowTrace
    0x42580,  // NOLINT: narrowToWideTtuStateRegFile
    0x42600,  // NOLINT: dmaNarrowToWideTrace
    0x42620,  // NOLINT: ringBusConsumer0TtuStateRegFile
    0x42640,  // NOLINT: dmaRingBusConsumer0Trace
    0x42660,  // NOLINT: ringBusConsumer1TtuStateRegFile
    0x42680,  // NOLINT: dmaRingBusConsumer1Trace
    0x426a0,  // NOLINT: ringBusProducerTtuStateRegFile
    0x426c0,  // NOLINT: dmaRingBusProducerTrace
    0x42700,  // NOLINT: meshBus0TtuStateRegFile
    0x42740,  // NOLINT: dmaMeshBus0Trace
    0x42780,  // NOLINT: meshBus1TtuStateRegFile
    0x427c0,  // NOLINT: dmaMeshBus1Trace
    0x42800,  // NOLINT: meshBus2TtuStateRegFile
    0x42840,  // NOLINT: dmaMeshBus2Trace
    0x42880,  // NOLINT: meshBus3TtuStateRegFile
    0x428c0,  // NOLINT: dmaMeshBus3Trace
};

const HibKernelCsrOffsets kBeagleHibKernelCsrOffsets = {
    0x46000,  // NOLINT: page_table_size
    0x46008,  // NOLINT: extended_table
    0x46050,  // NOLINT: dma_pause
    0x46078,  // NOLINT: page_table_init
    0x46080,  // NOLINT: msix_table_init
    0x50000,  // NOLINT: page_table
};

const HibUserCsrOffsets kBeagleHibUserCsrOffsets = {
    0x486b0,  // NOLINT: top_level_int_control
    0x486b8,  // NOLINT: top_level_int_status
    0x486d0,  // NOLINT: sc_host_int_count
    0x486d8,  // NOLINT: dma_pause
    0x486e0,  // NOLINT: dma_paused
    0x486e8,  // NOLINT: status_block_update
    0x486f0,  // NOLINT: hib_error_status
    0x486f8,  // NOLINT: hib_error_mask
    0x48700,  // NOLINT: hib_first_error_status
    0x48708,  // NOLINT: hib_first_error_timestamp
    0x48710,  // NOLINT: hib_inject_error
    0x487a8,  // NOLINT: dma_burst_limiter
};

const QueueCsrOffsets kBeagleInstructionQueueCsrOffsets = {
    0x48568,  // NOLINT: instruction_queue_control
    0x48570,  // NOLINT: instruction_queue_status
    0x48578,  // NOLINT: instruction_queue_descriptor_size
    0x48590,  // NOLINT: instruction_queue_base
    0x48598,  // NOLINT: instruction_queue_status_block_base
    0x485a0,  // NOLINT: instruction_queue_size
    0x485a8,  // NOLINT: instruction_queue_tail
    0x485b0,  // NOLINT: instruction_queue_fetched_head
    0x485b8,  // NOLINT: instruction_queue_completed_head
    0x485c0,  // NOLINT: instruction_queue_int_control
    0x485c8,  // NOLINT: instruction_queue_int_status
    0x48580,  // NOLINT: instruction_queue_minimum_size
    0x48588,  // NOLINT: instruction_queue_maximum_size
    0x46018,  // NOLINT: instruction_queue_int_vector
};

const MemoryCsrOffsets kBeagleMemoryMemoryCsrOffsets = {
    0x42010,  // NOLINT: memoryAccess
    0x42018,  // NOLINT: memoryData
};

const ScalarCoreCsrOffsets kBeagleScalarCoreCsrOffsets = {
    0x44018,  // NOLINT: scalarCoreRunControl
    0x44158,  // NOLINT: avDataPopRunControl
    0x44198,  // NOLINT: parameterPopRunControl
    0x441d8,  // NOLINT: infeedRunControl
    0x44218,  // NOLINT: outfeedRunControl
};

const MemoryCsrOffsets kBeagleScmemoryMemoryCsrOffsets = {
    0x44040,  // NOLINT: scMemoryAccess
    0x44048,  // NOLINT: scMemoryData
};

const TileConfigCsrOffsets kBeagleTileConfigCsrOffsets = {
    0x48788,  // NOLINT: tileconfig0
    0x48790,  // NOLINT: tileconfig1
};

const TileCsrOffsets kBeagleTileCsrOffsets = {
    0x400c0,                    // NOLINT: opRunControl
    static_cast<uint64_t>(-1),  // UNUSED, narrowToNarrowRunControl
    0x40150,                    // NOLINT: narrowToWideRunControl
    0x40110,                    // NOLINT: wideToNarrowRunControl
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_0
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_0
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_0
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_1
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_1
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_1
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_2
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_2
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_2
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_3
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_3
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_3
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_4
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_4
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_4
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_5
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_5
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_5
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_6
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_6
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_6
    static_cast<uint64_t>(-1),  // UNUSED, opRunControl_7
    static_cast<uint64_t>(-1),  // UNUSED, narrowToWideRunControl_7
    static_cast<uint64_t>(-1),  // UNUSED, wideToNarrowRunControl_7
    0x40190,                    // NOLINT: ringBusConsumer0RunControl
    0x401d0,                    // NOLINT: ringBusConsumer1RunControl
    0x40210,                    // NOLINT: ringBusProducerRunControl
    0x40250,                    // NOLINT: meshBus0RunControl
    0x40298,                    // NOLINT: meshBus1RunControl
    0x402e0,                    // NOLINT: meshBus2RunControl
    0x40328,                    // NOLINT: meshBus3RunControl
    0x40020,                    // NOLINT: deepSleep
};

const WireCsrOffsets kBeagleWireCsrOffsets = {
    0x48778,  // NOLINT: wire_int_pending_bit_array
    0x48780,  // NOLINT: wire_int_mask_array
};

const InterruptCsrOffsets kBeagleUsbFatalErrIntInterruptCsrOffsets = {
    0x4c060,  // NOLINT: fatal_err_int_control
    0x4c068,  // NOLINT: fatal_err_int_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt0InterruptCsrOffsets = {
    0x4c0b0,  // NOLINT: sc_host_int_0_control
    0x4c0b8,  // NOLINT: sc_host_int_0_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt1InterruptCsrOffsets = {
    0x4c0c8,  // NOLINT: sc_host_int_1_control
    0x4c0d0,  // NOLINT: sc_host_int_1_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt2InterruptCsrOffsets = {
    0x4c0e0,  // NOLINT: sc_host_int_2_control
    0x4c0e8,  // NOLINT: sc_host_int_2_status
};

const InterruptCsrOffsets kBeagleUsbScHostInt3InterruptCsrOffsets = {
    0x4c0f8,  // NOLINT: sc_host_int_3_control
    0x4c100,  // NOLINT: sc_host_int_3_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt0InterruptCsrOffsets = {
    0x4c070,  // NOLINT: top_level_int_0_control
    0x4c078,  // NOLINT: top_level_int_0_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt1InterruptCsrOffsets = {
    0x4c080,  // NOLINT: top_level_int_1_control
    0x4c088,  // NOLINT: top_level_int_1_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt2InterruptCsrOffsets = {
    0x4c090,  // NOLINT: top_level_int_2_control
    0x4c098,  // NOLINT: top_level_int_2_status
};

const InterruptCsrOffsets kBeagleUsbTopLevelInt3InterruptCsrOffsets = {
    0x4c0a0,  // NOLINT: top_level_int_3_control
    0x4c0a8,  // NOLINT: top_level_int_3_status
};

const ApexCsrOffsets kBeagleApexCsrOffsets = {
    0x1a000,  // NOLINT: omc0_00
    0x1a0d0,  // NOLINT: omc0_d0
    0x1a0d4,  // NOLINT: omc0_d4
    0x1a0d8,  // NOLINT: omc0_d8
    0x1a0dc,  // NOLINT: omc0_dc
    0x1a600,  // NOLINT: mst_abm_en
    0x1a500,  // NOLINT: slv_abm_en
    0x1a558,  // NOLINT: slv_err_resp_isr_mask
    0x1a658,  // NOLINT: mst_err_resp_isr_mask
    0x1a640,  // NOLINT: mst_wr_err_resp
    0x1a644,  // NOLINT: mst_rd_err_resp
    0x1a540,  // NOLINT: slv_wr_err_resp
    0x1a544,  // NOLINT: slv_rd_err_resp
    0x1a704,  // NOLINT: rambist_ctrl_1
    0x1a200,  // NOLINT: efuse_00
};

const CbBridgeCsrOffsets kBeagleCbBridgeCsrOffsets = {
    0x19018,  // NOLINT: bo0_fifo_status
    0x1901c,  // NOLINT: bo1_fifo_status
    0x19020,  // NOLINT: bo2_fifo_status
    0x19024,  // NOLINT: bo3_fifo_status
    0x1907c,  // NOLINT: gcbb_credit0
};

const MiscCsrOffsets kBeagleMiscCsrOffsets = {
    0x4a000,  // NOLINT: idleRegister
};

const MsixCsrOffsets kBeagleMsixCsrOffsets = {
    0x46018,  // NOLINT: instruction_queue_int_vector
    0x46020,  // NOLINT: input_actv_queue_int_vector
    0x46028,  // NOLINT: param_queue_int_vector
    0x46030,  // NOLINT: output_actv_queue_int_vector
    0x46040,  // NOLINT: top_level_int_vector
    0x46038,  // NOLINT: sc_host_int_vector
    0x46048,  // NOLINT: fatal_err_int_vector
    0x46068,  // NOLINT: msix_pending_bit_array0
    0x46800,  // NOLINT: msix_table
};

const ScuCsrOffsets kBeagleScuCsrOffsets = {
    0x1a30c,  // NOLINT: scu_ctrl_0
    0x1a310,  // NOLINT: scu_ctrl_1
    0x1a314,  // NOLINT: scu_ctrl_2
    0x1a318,  // NOLINT: scu_ctrl_3
    0x1a31c,  // NOLINT: scu_ctrl_4
    0x1a320,  // NOLINT: scu_ctrl_5
    0x1a32c,  // NOLINT: scu_ctr_6
    0x1a33c,  // NOLINT: scu_ctr_7
};

const UsbCsrOffsets kBeagleUsbCsrOffsets = {
    0x4c058,  // NOLINT: outfeed_chunk_length
    0x4c148,  // NOLINT: descr_ep
    0x4c150,  // NOLINT: ep_status_credit
    0x4c160,  // NOLINT: multi_bo_ep
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CSR_OFFSETS_H_
