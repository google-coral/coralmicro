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

#include "libs/base/utils.h"

#include <vector>

#include "libs/base/filesystem.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/ip_addr.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/wwd_constants.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include/wiced_defaults.h"

namespace coralmicro {

uint64_t GetUniqueId() {
  uint32_t fuse_val_hi, fuse_val_lo;
  fuse_val_lo = OCOTP->FUSEN[16].FUSE;
  fuse_val_hi = OCOTP->FUSEN[17].FUSE;
  return ((static_cast<uint64_t>(fuse_val_hi) << 32) | fuse_val_lo);
}

std::string GetSerialNumber() {
  uint64_t id = GetUniqueId();
  char serial[17];  // 16 hex characters + \0
  snprintf(serial, sizeof(serial), "%08lx%08lx",
           static_cast<uint32_t>(id >> 32), static_cast<uint32_t>(id));
  return serial;
}

bool GetIpFromFile(const char* path, ip4_addr_t* addr) {
  std::string ip_str;
  if (LfsReadFile(path, &ip_str)) {
    if (ipaddr_aton(ip_str.c_str(), addr)) {
      return true;
    }
  }
  return false;
}

bool GetUsbIpAddress(std::string* usb_ip_out) {
  return coralmicro::LfsReadFile("/usb_ip_address", usb_ip_out);
}

extern "C" wiced_country_code_t coral_micro_get_wiced_country_code(void) {
  std::string wifi_country_code_out, wifi_revision_out;
  unsigned short wifi_revision = 0;
  if (!coralmicro::LfsReadFile("/wifi_country", &wifi_country_code_out)) {
    DbgConsole_Printf("failed to read back country, returning default\r\n");
    return WICED_DEFAULT_COUNTRY_CODE;
  }
  if (wifi_country_code_out.length() != 2) {
    DbgConsole_Printf("wifi_country must be 2 bytes, returning default\r\n");
    return WICED_DEFAULT_COUNTRY_CODE;
  }
  if (coralmicro::LfsReadFile("/wifi_revision", &wifi_revision_out)) {
    wifi_revision = *reinterpret_cast<uint16_t*>(wifi_revision_out.data());
  }
  return static_cast<wiced_country_code_t>(MK_CNTRY(
      wifi_country_code_out[0], wifi_country_code_out[1], wifi_revision));
}

}  // namespace coralmicro
