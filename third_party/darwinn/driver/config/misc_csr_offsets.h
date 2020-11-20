#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_MISC_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_MISC_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for custom block misc.
// Members are intentionally named to match the GCSR register names.
struct MiscCsrOffsets {
  uint64 idleRegister;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_MISC_CSR_OFFSETS_H_
