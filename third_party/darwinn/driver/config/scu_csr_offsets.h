#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_SCU_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_SCU_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for programming the SCU in Beagle
// (the block containing state machines to control boot and power sequences)
// Members are intentionally named to match the GCSR register names.
struct ScuCsrOffsets {
  // The SCU control registers have generic names but each contain many small
  // fields which are reflected in the spec (should use csr_helper to access)
  uint64 scu_ctrl_0;
  uint64 scu_ctrl_1;
  uint64 scu_ctrl_2;
  uint64 scu_ctrl_3;
  uint64 scu_ctrl_4;
  uint64 scu_ctrl_5;
  uint64 scu_ctr_6;
  uint64 scu_ctr_7;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_SCU_CSR_OFFSETS_H_
