#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_WIRE_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_WIRE_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for handling wire interrupts.
// Members are intentionally named to match the GCSR register names.
struct WireCsrOffsets {
  // Tells which interrupts should be serviced.
  uint64 wire_int_pending_bit_array;
  uint64 wire_int_mask_array;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_WIRE_CSR_OFFSETS_H_
