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

#ifndef LIBS_BASE_ETHERNET_H_
#define LIBS_BASE_ETHERNET_H_

#include <optional>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/netifapi.h"

namespace coralmicro {

// Gets the ethernet interface, which contains info like
// ip and hw addresses, interface names, etc.
//
// @return A pointer to the netif ethernet interface, or nullptr if
// `EthernetInit()` has not been called or the POE add-on board failed to
// initialize. See:
// https://os.mbed.com/docs/mbed-os/v6.15/mbed-os-api-doxy/structnetif.html
struct netif* EthernetGetInterface();

// Initializes the ethernet module.
//
// This function requires that the POE add-on board is connected and it must be
// called before `EthernetGetInterface()`.
// @param default_iface True sets ethernet as the default network interface,
// false disables it.
void EthernetInit(bool default_iface);

// @cond Do not generate docs.
// Writes data over the SMI to the specified PHY register.
//
// @param phy_reg The PHY register to write to.
// @param data The data to write into that register.
// @return A status code, see:
// https://mcuxpresso.nxp.com/api_doc/dev/2349/a00344.html#ga7ff0b98bb1341c07acefb1473b6eda29
status_t EthernetPHYWrite(uint32_t phy_reg, uint32_t data);
// @endcond

// Gets the device's Ethernet IP address.
//
// @return A string representing the IPv4 IP address or `std::nullopt` on
// failure.
std::optional<std::string> EthernetGetIp();

// Retrieves the ethernet speed that is stored in flash memory.
// @returns The ethernet speed in Mbps.
//   The default return value is 100, if no value is stored in flash.
int EthernetGetSpeed();

}  // namespace coralmicro

#endif  // LIBS_BASE_ETHERNET_H_
