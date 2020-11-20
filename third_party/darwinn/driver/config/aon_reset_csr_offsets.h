#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_AON_RESET_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_AON_RESET_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for aon reset. Members are
// intentionally named to match the GCSR register names.
struct AonResetCsrOffsets {
  // Resets CB.
  uint64 resetReg;

  // Clock enable register.
  uint64 clockEnableReg;

  // Shutdown registers.
  uint64 logicShutdownReg;
  uint64 logicShutdownPreReg;
  uint64 logicShutdownAllReg;
  uint64 memoryShutdownReg;
  uint64 memoryShutdownAckReg;
  uint64 tileMemoryRetnReg;

  // Clamp register.
  uint64 clampEnableReg;

  // Quiesce register.
  uint64 forceQuiesceReg;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_AON_RESET_CSR_OFFSETS_H_
