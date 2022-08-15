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

#include "libs/base/ntp.h"

#include "libs/base/check.h"
#include "libs/base/timer.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/sntp.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/tcpip.h"

namespace coralmicro {

void NtpInit() {
  tcpip_callback(
      [](void*) -> void {
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "0.pool.ntp.org");
        sntp_setservername(1, "1.pool.ntp.org");
        sntp_setservername(2, "2.pool.ntp.org");
        sntp_setservername(3, "3.pool.ntp.org");
        sntp_init();
      },
      nullptr);
}

}  // namespace coralmicro

extern "C" void sntp_set_system_time(uint32_t sec) {
  coralmicro::TimerSetRtcTime(sec);
}