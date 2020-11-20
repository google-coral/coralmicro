#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_SYNC_FLAG_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_SYNC_FLAG_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for accessing sync flags for scalar
// core and tiles.
struct SyncFlagCsrOffsets {
  // Intentionally named to match the GCSR register names.
  uint64 SyncCounter;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_SYNC_FLAG_CSR_OFFSETS_H_
