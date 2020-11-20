#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_TILE_CONFIG_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_TILE_CONFIG_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for configuring indirect tile accesses.
// Members are intentionally named to match the GCSR register names.
struct TileConfigCsrOffsets {
  // Used only by driver.
  uint64 tileconfig0;

  // Used by debugger, and other purposes.
  uint64 tileconfig1;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_TILE_CONFIG_CSR_OFFSETS_H_
