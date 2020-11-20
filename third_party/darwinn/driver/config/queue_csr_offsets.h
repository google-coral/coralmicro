#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_

#include <stddef.h>

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds the various CSR offsets for programming queue behaviors.
// Members are intentionally named to match the GCSR register names.
struct QueueCsrOffsets {
  uint64 queue_control;
  uint64 queue_status;
  uint64 queue_descriptor_size;
  uint64 queue_base;
  uint64 queue_status_block_base;
  uint64 queue_size;
  uint64 queue_tail;
  uint64 queue_fetched_head;
  uint64 queue_completed_head;
  uint64 queue_int_control;
  uint64 queue_int_status;
  uint64 queue_minimum_size;
  uint64 queue_maximum_size;
  uint64 queue_int_vector;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_
