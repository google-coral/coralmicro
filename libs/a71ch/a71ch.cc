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

#include "libs/a71ch/a71ch.h"

#include "libs/base/gpio.h"
#include "third_party/a71ch/hostlib/hostLib/inc/a71ch_api.h"
#include "third_party/a71ch/sss/ex/inc/ex_sss_boot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro::a71ch {
namespace {
// Helper function to convert an array of unsigned char to a string in hex
// value.
auto to_hex = [](const uint8_t* src, uint16_t src_len) -> std::string {
  std::string output;
  output.reserve(src_len * 2);
  for (int dest_idx = 0, src_idx = 0; src_idx < src_len;
       dest_idx += 2, src_idx += 1) {
    sprintf(&output[dest_idx], "%02x", src[src_idx]);
  }
  return output;
};
}  // namespace

bool Init() {
  coralmicro::gpio::SetGpio(coralmicro::gpio::kCryptoRst, false);
  vTaskDelay(pdMS_TO_TICKS(1));
  coralmicro::gpio::SetGpio(coralmicro::gpio::kCryptoRst, true);

  SE_Connect_Ctx_t sessionCtxt = {0};
  sss_status_t status = sss_session_open(
      nullptr, kType_SSS_SE_A71CH, 0, kSSS_ConnectionType_Plain, &sessionCtxt);
  if (kStatus_SSS_Success != status) {
    return false;
  }
  return true;
}

std::optional<std::string> GetUID() {
  uint16_t uid_len = A71CH_MODULE_UNIQUE_ID_LEN;
  uint8_t uid[uid_len];
  if (A71_GetUniqueID(uid, &uid_len) != SMCOM_OK) {
    return std::nullopt;
  }
  return to_hex(uid, uid_len);
}
}  // namespace coralmicro::a71ch
