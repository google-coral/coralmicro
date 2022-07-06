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

namespace coralmicro::utils {

uint64_t GetUniqueID() {
    uint32_t fuse_val_hi, fuse_val_lo;
    fuse_val_lo = OCOTP->FUSEN[16].FUSE;
    fuse_val_hi = OCOTP->FUSEN[17].FUSE;
    return ((static_cast<uint64_t>(fuse_val_hi) << 32) | fuse_val_lo);
}

std::string GetSerialNumber() {
    uint64_t id = coralmicro::utils::GetUniqueID();
    char serial[17];  // 16 hex characters + \0
    snprintf(serial, sizeof(serial), "%08lx%08lx",
             static_cast<uint32_t>(id >> 32), static_cast<uint32_t>(id));
    return serial;
}

std::array<uint8_t, 6> GetMacAddress() {
    uint32_t fuse_val_hi, fuse_val_lo;
    fuse_val_lo = OCOTP->FUSEN[FUSE_ADDRESS_TO_OCOTP_INDEX(MAC1_ADDR_LO)].FUSE;
    fuse_val_hi =
        OCOTP->FUSEN[FUSE_ADDRESS_TO_OCOTP_INDEX(MAC1_ADDR_HI)].FUSE & 0xFFFF;
    uint8_t a = (fuse_val_hi >> 8) & 0xFF;
    uint8_t b = (fuse_val_hi)&0xFF;
    uint8_t c = (fuse_val_lo >> 24) & 0xFF;
    uint8_t d = (fuse_val_lo >> 16) & 0xFF;
    uint8_t e = (fuse_val_lo >> 8) & 0xFF;
    uint8_t f = (fuse_val_lo)&0xFF;
    return std::array<uint8_t, 6>{a, b, c, d, e, f};
}

bool GetUSBIPAddress(ip4_addr_t* usb_ip_out) {
    std::string usb_ip;
    if (!GetUSBIPAddress(&usb_ip)) return false;
    if (!ipaddr_aton(usb_ip.c_str(), usb_ip_out)) return false;
    return true;
}

bool GetUSBIPAddress(std::string* usb_ip_out) {
    return coralmicro::filesystem::ReadFile("/usb_ip_address", usb_ip_out);
}

bool SetWiFiSSID(std::string* wifi_ssid) {
    return coralmicro::filesystem::WriteFile(
        "/wifi_ssid", reinterpret_cast<const uint8_t*>(wifi_ssid->c_str()),
        wifi_ssid->size());
}

bool GetWiFiSSID(std::string* wifi_ssid_out) {
    return coralmicro::filesystem::ReadFile("/wifi_ssid", wifi_ssid_out);
}

bool SetWiFiPSK(std::string* wifi_psk) {
    return coralmicro::filesystem::WriteFile(
        "/wifi_psk", reinterpret_cast<const uint8_t*>(wifi_psk->c_str()),
        wifi_psk->size());
}

bool GetWiFiPSK(std::string* wifi_psk_out) {
    return coralmicro::filesystem::ReadFile("/wifi_psk", wifi_psk_out);
}

int GetEthernetSpeed() {
    std::string ethernet_speed;
    uint16_t speed;
    if (!coralmicro::filesystem::ReadFile("/ethernet_speed",
                                            &ethernet_speed)) {
        printf("Failed to read ethernet speed, assuming 100M.\r\n");
        speed = 100;
    } else {
        speed = *reinterpret_cast<uint16_t*>(ethernet_speed.data());
    }
    return speed;
}

extern "C" wiced_country_code_t coral_micro_get_wiced_country_code(void) {
    std::string wifi_country_code_out, wifi_revision_out;
    unsigned short wifi_revision = 0;
    if (!coralmicro::filesystem::ReadFile("/wifi_country",
                                            &wifi_country_code_out)) {
        DbgConsole_Printf("failed to read back country, returning WW\r\n");
        return WICED_COUNTRY_WORLD_WIDE_XX;
    }
    if (wifi_country_code_out.length() != 2) {
        DbgConsole_Printf("wifi_country must be 2 bytes, returning WW\r\n");
        return WICED_COUNTRY_WORLD_WIDE_XX;
    }
    if (coralmicro::filesystem::ReadFile("/wifi_revision",
                                           &wifi_revision_out)) {
        wifi_revision = *reinterpret_cast<uint16_t*>(wifi_revision_out.data());
    }
    return static_cast<wiced_country_code_t>(MK_CNTRY(
        wifi_country_code_out[0], wifi_country_code_out[1], wifi_revision));
}

}  // namespace coralmicro::utils
