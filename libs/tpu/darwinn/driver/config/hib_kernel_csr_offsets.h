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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_HIB_KERNEL_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_HIB_KERNEL_CSR_OFFSETS_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for kernel space in HIB. Members are
// intentionally named to match the GCSR register names.
struct HibKernelCsrOffsets {
  uint64_t page_table_size;
  uint64_t extended_table;
  uint64_t dma_pause;

  // Tracks whether initialization is done.
  uint64_t page_table_init;
  uint64_t msix_table_init;

  // Points to the first entry in the page table. Subsequent entries can be
  // accessed with increasing offsets if they exist.
  uint64_t page_table;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_HIB_KERNEL_CSR_OFFSETS_H_
