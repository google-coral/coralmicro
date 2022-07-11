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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for tiles. Members are intentionally
// named to match the GCSR register names.
struct TileCsrOffsets {
  // RunControls to change run state.
  uint64_t opRunControl;
  uint64_t narrowToNarrowRunControl;
  uint64_t narrowToWideRunControl;
  uint64_t wideToNarrowRunControl;
  // When we enable the wider thread issue feature, we get multiple
  // of these run controls per pipeline for the opcontrol, narrow to wide
  // and wide to narrow. We're using 8 of these as a maximum issue width
  // at this point. The driver will only use the registers that are valid
  // for any given configuration.
  // TODO(b/119571140)
  uint64_t opRunControl_0;
  uint64_t narrowToWideRunControl_0;
  uint64_t wideToNarrowRunControl_0;
  uint64_t opRunControl_1;
  uint64_t narrowToWideRunControl_1;
  uint64_t wideToNarrowRunControl_1;
  uint64_t opRunControl_2;
  uint64_t narrowToWideRunControl_2;
  uint64_t wideToNarrowRunControl_2;
  uint64_t opRunControl_3;
  uint64_t narrowToWideRunControl_3;
  uint64_t wideToNarrowRunControl_3;
  uint64_t opRunControl_4;
  uint64_t narrowToWideRunControl_4;
  uint64_t wideToNarrowRunControl_4;
  uint64_t opRunControl_5;
  uint64_t narrowToWideRunControl_5;
  uint64_t wideToNarrowRunControl_5;
  uint64_t opRunControl_6;
  uint64_t narrowToWideRunControl_6;
  uint64_t wideToNarrowRunControl_6;
  uint64_t opRunControl_7;
  uint64_t narrowToWideRunControl_7;
  uint64_t wideToNarrowRunControl_7;
  uint64_t ringBusConsumer0RunControl;
  uint64_t ringBusConsumer1RunControl;
  uint64_t ringBusProducerRunControl;
  uint64_t meshBus0RunControl;
  uint64_t meshBus1RunControl;
  uint64_t meshBus2RunControl;
  uint64_t meshBus3RunControl;

  // Deep sleep register to control power state.
  uint64_t deepSleep;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_
