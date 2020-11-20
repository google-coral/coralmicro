// AUTO GENERATED FILE.
// See http://go/darwinn-chip-structure for more info.

#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_

#include "third_party/darwinn/driver/config/chip_structures.h"

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

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_BEAGLE_BEAGLE_CHIP_STRUCTURES_H_
