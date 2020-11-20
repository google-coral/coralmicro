#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for tiles. Members are intentionally
// named to match the GCSR register names.
struct TileCsrOffsets {
  // RunControls to change run state.
  uint64 opRunControl;
  uint64 narrowToNarrowRunControl;
  uint64 narrowToWideRunControl;
  uint64 wideToNarrowRunControl;
  // When we enable the wider thread issue feature, we get multiple
  // of these run controls per pipeline for the opcontrol, narrow to wide
  // and wide to narrow. We're using 8 of these as a maximum issue width
  // at this point. The driver will only use the registers that are valid
  // for any given configuration.
  // TODO(b/119571140)
  uint64 opRunControl_0;
  uint64 narrowToWideRunControl_0;
  uint64 wideToNarrowRunControl_0;
  uint64 opRunControl_1;
  uint64 narrowToWideRunControl_1;
  uint64 wideToNarrowRunControl_1;
  uint64 opRunControl_2;
  uint64 narrowToWideRunControl_2;
  uint64 wideToNarrowRunControl_2;
  uint64 opRunControl_3;
  uint64 narrowToWideRunControl_3;
  uint64 wideToNarrowRunControl_3;
  uint64 opRunControl_4;
  uint64 narrowToWideRunControl_4;
  uint64 wideToNarrowRunControl_4;
  uint64 opRunControl_5;
  uint64 narrowToWideRunControl_5;
  uint64 wideToNarrowRunControl_5;
  uint64 opRunControl_6;
  uint64 narrowToWideRunControl_6;
  uint64 wideToNarrowRunControl_6;
  uint64 opRunControl_7;
  uint64 narrowToWideRunControl_7;
  uint64 wideToNarrowRunControl_7;
  uint64 ringBusConsumer0RunControl;
  uint64 ringBusConsumer1RunControl;
  uint64 ringBusProducerRunControl;
  uint64 meshBus0RunControl;
  uint64 meshBus1RunControl;
  uint64 meshBus2RunControl;
  uint64 meshBus3RunControl;

  // Deep sleep register to control power state.
  uint64 deepSleep;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_TILE_CSR_OFFSETS_H_
