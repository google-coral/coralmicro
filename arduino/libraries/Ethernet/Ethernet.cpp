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

#include "Ethernet.h"

#include <cstring>

#include "libs/base/check.h"
#include "libs/base/ethernet.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/dns.h"

namespace coralmicro {
namespace arduino {

int EthernetClass::begin() {
  EthernetInit(/*default_iface=*/true);
  auto ip_address = EthernetGetIp();
  if (ip_address.has_value()) {
    CHECK(ip_address_.fromString(ip_address.value().c_str()));

    // We know DHCP succeeded, so populate the rest of our cached data.
    auto subnet_mask = EthernetGetSubnetMask();
    CHECK(subnet_mask_.fromString(subnet_mask.value().c_str()));

    auto gateway = EthernetGetGateway();
    CHECK(gateway_.fromString(gateway.value().c_str()));

    dns_server_ = IPAddress(dns_getserver(0)->addr);
    link_on_ = true;
    return 1;
  }
  return 0;
}

IPAddress EthernetClass::localIP() { return ip_address_; }

IPAddress EthernetClass::subnetMask() { return subnet_mask_; }

IPAddress EthernetClass::gatewayIP() { return gateway_; }

IPAddress EthernetClass::dnsServerIP() { return dns_server_; }

void EthernetClass::MACAddress(uint8_t* mac) {
  auto mac_array = EthernetGetMacAddress();
  std::memcpy(mac, mac_array.data(), 6);
}

EthernetLinkStatus EthernetClass::linkStatus() {
  return link_on_ ? LinkON : LinkOFF;
}

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::EthernetClass Ethernet;