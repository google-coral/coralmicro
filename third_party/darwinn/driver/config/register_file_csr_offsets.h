#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_REGISTER_FILE_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_REGISTER_FILE_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds CSR offsets for accessing register file. Members are
// intentionally named to match the GCSR register names.
struct RegisterFileCsrOffsets {
  // Points to first register in the register file. Subsequent registers can be
  // accessed with increasing offsets if they exist.
  uint64 RegisterFile;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_REGISTER_FILE_CSR_OFFSETS_H_
