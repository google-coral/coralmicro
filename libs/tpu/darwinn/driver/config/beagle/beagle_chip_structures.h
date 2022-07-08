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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_

#include "libs/tpu/darwinn/driver/config/chip_structures.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

const ChipStructures kBeagleChipStructures = {
    8ULL,     // NOLINT: minimum_alignment_bytes
    4096ULL,  // NOLINT: allocation_alignment_bytes
    0ULL,     // NOLINT: axi_dma_burst_limiter
    0ULL,     // NOLINT: num_wire_interrupts
    8192ULL,  // NOLINT: num_page_table_entries
    64ULL,    // NOLINT: physical_address_bits
    0ULL,     // NOLINT: tpu_dram_size_bytes
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_
