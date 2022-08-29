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

#ifndef LIBS_BASE_UTILS_H_
#define LIBS_BASE_UTILS_H_

#include <array>
#include <cstdint>
#include <string>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/ip_addr.h"

#define FUSE_ADDRESS_TO_OCOTP_INDEX(fuse) ((fuse - 0x800) >> 4)
#define MAC1_ADDR_LO (0xA80)
#define MAC1_ADDR_HI (0xA90)

namespace coralmicro {

// Gets the 64-bit unique identifiers of the RT1176.
// @returns 64-bit value that is unique to the device.
uint64_t GetUniqueId();

// Gets the hex string representation of the unique identifier.
// @returns String containing the 64-bit unique identifier, as a printable hex
// string.
std::string GetSerialNumber();

// Gets the USB IP address that is stored in flash memory.
// @param usb_ip_out A pointer to a string in which to store a printable version
// of the IP.
// @returns True if the address was successfully retrieved; false otherwise.
bool GetUsbIpAddress(std::string* usb_ip_out);

// Attempts to open a given file at `path`, and if the contents are an
// IP address, return them in `addr`.
//
// @param path File path to read an address from.
// @param addr Pointer to an `ip4_addr_t` that the address will be stored in.
// @returns True if successful; false otherwise.
bool GetIpFromFile(const char* path, ip4_addr_t* addr);

}  // namespace coralmicro

#endif  // LIBS_BASE_UTILS_H_
