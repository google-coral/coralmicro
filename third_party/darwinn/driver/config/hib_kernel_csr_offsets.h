#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_HIB_KERNEL_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_HIB_KERNEL_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for kernel space in HIB. Members are
// intentionally named to match the GCSR register names.
struct HibKernelCsrOffsets {
  uint64 page_table_size;
  uint64 extended_table;
  uint64 dma_pause;

  // Tracks whether initialization is done.
  uint64 page_table_init;
  uint64 msix_table_init;

  // Points to the first entry in the page table. Subsequent entries can be
  // accessed with increasing offsets if they exist.
  uint64 page_table;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_HIB_KERNEL_CSR_OFFSETS_H_
