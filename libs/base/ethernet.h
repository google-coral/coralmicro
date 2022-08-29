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

#include <array>
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
// @return True if ethernet successfully initialized; false otherwise.
bool EthernetInit(bool default_iface);

// @cond Do not generate docs.
// Writes data over the SMI to the specified PHY register.
//
// @param phy_reg The PHY register to write to.
// @param data The data to write into that register.
// @return A status code, see:
// https://mcuxpresso.nxp.com/api_doc/dev/2349/a00344.html#ga7ff0b98bb1341c07acefb1473b6eda29
status_t EthernetPhyWrite(uint32_t phy_reg, uint32_t data);
// @endcond

// Gets the device's Ethernet IP address, with a timeout of 30s.
//
// @return A string representing the IPv4 IP address or `std::nullopt` on
// failure.
std::optional<std::string> EthernetGetIp();

// Gets the device's Ethernet IP address.
//
// @param timeout_ms Amount of time to wait for DHCP to finish, in milliseconds.
// @return A string representing the IPv4 IP address or `std::nullopt` on
// failure.
std::optional<std::string> EthernetGetIp(uint64_t timeout_ms);

// Gets the assigned MAC address from the device fuses.
// @returns The MAC address assigned to the device.
std::array<uint8_t, 6> EthernetGetMacAddress();

std::optional<std::string> EthernetGetSubnetMask();
std::optional<std::string> EthernetGetSubnetMask(uint64_t timeout_ms);
std::optional<std::string> EthernetGetGateway();
std::optional<std::string> EthernetGetGateway(uint64_t timeout_ms);

// Retrieves the ethernet speed that is stored in flash memory.
//
// @returns The ethernet speed in Mbps.
//   The default return value is 100, if no value is stored in flash.
int EthernetGetSpeed();

// Stores an IP address in flash memory, to be used as the Ethernet IP.
//
// @param addr IP address to store.
// @returns True if the address was stored successfully; false otherwise.
bool EthernetSetStaticIp(ip4_addr_t addr);

// Stores an IP address in flash memory, to be used as the Ethernet subnet mask.
//
// @param addr IP address to store.
// @returns True if the address was stored successfully; false otherwise.
bool EthernetSetStaticSubnetMask(ip4_addr_t addr);

// Stores an IP address in flash memory, to be used as the Ethernet gateway
// address.
//
// @param addr IP address to store.
// @returns True if the address was stored successfully; false otherwise.
bool EthernetSetStaticGateway(ip4_addr_t addr);

}  // namespace coralmicro

#endif  // LIBS_BASE_ETHERNET_H_
