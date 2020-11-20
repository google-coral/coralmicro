#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_SCALAR_CORE_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_SCALAR_CORE_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for scalar core.
// Members are intentionally named to match the GCSR register names.
struct ScalarCoreCsrOffsets {
  // RunControls.
  uint64 scalarCoreRunControl;
  uint64 avDataPopRunControl;
  uint64 parameterPopRunControl;
  uint64 infeedRunControl;
  uint64 outfeedRunControl;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_SCALAR_CORE_CSR_OFFSETS_H_
