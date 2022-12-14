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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_CHIP_STRUCTURES_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_CHIP_STRUCTURES_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various values that is derived from system_builder.proto.
struct ChipStructures {
  // Hardware required minimum alignment on buffers.
  uint64_t minimum_alignment_bytes;

  // Buffer allocation alignment and granularity. Typically this would be same
  // as minimum_alignment_bytes above, however may also factor in other
  // requirements such as host cache line size, cache API constraints etc.
  uint64_t allocation_alignment_bytes;

  // Controls AXI burst length.
  uint64_t axi_dma_burst_limiter;

  // Number of wire interrupts.
  uint64_t num_wire_interrupts;

  // Number of page table entries.
  uint64_t num_page_table_entries;

  // Number of physical address bits generated by the hardware.
  uint64_t physical_address_bits;

  // Addressable byte size of TPU DRAM (if any). This must be divisible by host
  // table size.
  uint64_t tpu_dram_size_bytes;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_CHIP_STRUCTURES_H_
