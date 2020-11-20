#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_HIB_USER_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_HIB_USER_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for user space in HIB. Members are
// intentionally named to match the GCSR register names.
struct HibUserCsrOffsets {
  // Interrupt control and status for top level.
  uint64 top_level_int_control;
  uint64 top_level_int_status;

  // Interrupt count for scalar core.
  uint64 sc_host_int_count;

  // DMA pauses.
  uint64 dma_pause;
  uint64 dma_paused;

  // Enable/disable status block update.
  uint64 status_block_update;

  // HIB errors.
  uint64 hib_error_status;
  uint64 hib_error_mask;
  uint64 hib_first_error_status;
  uint64 hib_first_error_timestamp;
  uint64 hib_inject_error;

  // Limits AXI DMA burst.
  uint64 dma_burst_limiter;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_HIB_USER_CSR_OFFSETS_H_
