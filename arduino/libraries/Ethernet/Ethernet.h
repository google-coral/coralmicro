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

#ifndef Ethernet_h
#define Ethernet_h

#include <Arduino.h>
#include "SocketClient.h"
#include "SocketServer.h"

// Status of the Ethernet link.
enum EthernetLinkStatus { Unknown, LinkON, LinkOFF };

namespace coralmicro {
namespace arduino {

using EthernetClient = SocketClient;
using EthernetServer = SocketServer;
// Allows for connection of the device to a network via Ethernet.
// At the moment, this interface only supports networks with DHCP.
// After a connection is established, information about the connection
// such as IP address, DNS, and gateway can be retrieved.
// You should not initialize this object yourself, instead include `Ethernet.h`
// and use the global `Ethernet` instance.
class EthernetClass {
 public:
  // Variant of begin that only uses DHCP.
  //
  // @returns 1 if DHCP was successful; 0 otherwise.
  int begin();

  // Returns the IP address of the device.
  //
  // @returns The Ethernet IP address of the device.
  IPAddress localIP();

  // Returns the subnet mask of Ethernet connection.
  //
  // @returns The subnet mask of the connection.
  IPAddress subnetMask();

  // Returns the gateway of the Ethernet connection.
  //
  // @returns The IP of the network gateway.
  IPAddress gatewayIP();

  // Returns the primary DNS server of the device.
  //
  // @returns The IP of the primary DNS server.
  IPAddress dnsServerIP();

  // Retrieves the MAC address of the Ethernet interface.
  //
  // @param mac Pointer to memory where the MAC address will be stored.
  void MACAddress(uint8_t* mac);

  // Returns the status of the Ethernet connection.
  //
  // @returns `EthernetLinkStatus` representing the state of the connection.
  EthernetLinkStatus linkStatus();

 private:
  IPAddress ip_address_;
  IPAddress subnet_mask_;
  IPAddress gateway_;
  IPAddress dns_server_;
  bool link_on_ = false;
};

}  // namespace arduino
}  // namespace coralmicro

extern coralmicro::arduino::EthernetClass Ethernet;

#endif  // Ethernet_h