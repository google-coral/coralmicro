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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_APEX_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_APEX_CSR_OFFSETS_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets for apex in Beagle.
// Members are intentionally named to match the GCSR register names.
struct ApexCsrOffsets {
  uint64_t omc0_00;

  uint64_t omc0_d0;
  uint64_t omc0_d4;
  uint64_t omc0_d8;
  uint64_t omc0_dc;

  uint64_t mst_abm_en;
  uint64_t slv_abm_en;
  uint64_t slv_err_resp_isr_mask;
  uint64_t mst_err_resp_isr_mask;

  uint64_t mst_wr_err_resp;
  uint64_t mst_rd_err_resp;
  uint64_t slv_wr_err_resp;
  uint64_t slv_rd_err_resp;

  uint64_t rambist_ctrl_1;

  uint64_t efuse_00;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_APEX_CSR_OFFSETS_H_
