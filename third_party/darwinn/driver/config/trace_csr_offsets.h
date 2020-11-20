#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_TRACE_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_TRACE_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for performance tracing.
// Members are intentionally named to match the GCSR register names.
struct TraceCsrOffsets {
  uint64 OverwriteMode;
  uint64 EnableTracing;
  uint64 Trace;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_TRACE_CSR_OFFSETS_H_
