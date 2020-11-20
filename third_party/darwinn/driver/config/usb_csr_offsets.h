#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_USB_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_USB_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for USB HIB.
// Members are intentionally named to match the GCSR register names.
struct UsbCsrOffsets {
  uint64 outfeed_chunk_length;
  uint64 descr_ep;
  uint64 ep_status_credit;
  uint64 multi_bo_ep;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_USB_CSR_OFFSETS_H_
