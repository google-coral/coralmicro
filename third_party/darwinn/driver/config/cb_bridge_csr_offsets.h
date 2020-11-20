#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_CB_BRIDGE_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_CB_BRIDGE_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for cb_bridge in Beagle.
// Members are intentionally named to match the GCSR register names.
struct CbBridgeCsrOffsets {
  uint64 bo0_fifo_status;
  uint64 bo1_fifo_status;
  uint64 bo2_fifo_status;
  uint64 bo3_fifo_status;
  uint64 gcbb_credit0;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_CB_BRIDGE_CSR_OFFSETS_H_
