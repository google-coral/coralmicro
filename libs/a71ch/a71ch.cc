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
#include "third_party/a71ch/sss/ex/inc/ex_sss_boot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coral::micro {
namespace a71ch {
bool Init() {
  coral::micro::gpio::SetGpio(coral::micro::gpio::kCryptoRst, false);
  vTaskDelay(pdMS_TO_TICKS(1));
  coral::micro::gpio::SetGpio(coral::micro::gpio::kCryptoRst, true);

  SE_Connect_Ctx_t sessionCtxt = {0};
  sss_status_t status = sss_session_open(nullptr, kType_SSS_SE_A71CH, 0, kSSS_ConnectionType_Plain, &sessionCtxt);
  if (kStatus_SSS_Success != status) {
    return false;
  }
  return true;
}
}  // namespace a71ch
}  // namespace coral::micro
