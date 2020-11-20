#ifndef THIRD_PARTY_DARWINN_DRIVER_CONFIG_APEX_CSR_OFFSETS_H_
#define THIRD_PARTY_DARWINN_DRIVER_CONFIG_APEX_CSR_OFFSETS_H_

#include "third_party/darwinn/port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for apex in Beagle.
// Members are intentionally named to match the GCSR register names.
struct ApexCsrOffsets {
  uint64 omc0_00;

  uint64 omc0_d4;
  uint64 omc0_d8;
  uint64 omc0_dc;

  uint64 mst_abm_en;
  uint64 slv_abm_en;
  uint64 slv_err_resp_isr_mask;
  uint64 mst_err_resp_isr_mask;

  uint64 mst_wr_err_resp;
  uint64 mst_rd_err_resp;
  uint64 slv_wr_err_resp;
  uint64 slv_rd_err_resp;

  uint64 rambist_ctrl_1;

  uint64 efuse_00;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // THIRD_PARTY_DARWINN_DRIVER_CONFIG_APEX_CSR_OFFSETS_H_
