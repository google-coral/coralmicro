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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_BREAKPOINT_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_BREAKPOINT_CSR_OFFSETS_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for breakpointing a hardware component.
// e.g. scalar core, TTU, etc.
// Members are intentionally named to match the GCSR register names.
struct BreakpointCsrOffsets {
  uint64_t RunControl;
  uint64_t RunStatus;
  uint64_t BreakPoint;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_BREAKPOINT_CSR_OFFSETS_H_
