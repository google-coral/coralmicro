/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_

#include <stddef.h>

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds the various CSR offsets for programming queue behaviors.
// Members are intentionally named to match the GCSR register names.
struct QueueCsrOffsets {
  uint64_t queue_control;
  uint64_t queue_status;
  uint64_t queue_descriptor_size;
  uint64_t queue_base;
  uint64_t queue_status_block_base;
  uint64_t queue_size;
  uint64_t queue_tail;
  uint64_t queue_fetched_head;
  uint64_t queue_completed_head;
  uint64_t queue_int_control;
  uint64_t queue_int_status;
  uint64_t queue_minimum_size;
  uint64_t queue_maximum_size;
  uint64_t queue_int_vector;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_QUEUE_CSR_OFFSETS_H_
