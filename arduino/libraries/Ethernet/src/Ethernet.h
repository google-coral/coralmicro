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

#ifndef CORAL_MICRO_POE
#error Missing Coral PoE Add-on libraries. Your board must be set to "Dev Board Micro + PoE Add-on"
#endif

#include <Arduino.h>

#include "SocketClient.h"
#include "SocketServer.h"

// Status of the Ethernet link.
enum EthernetLinkStatus { Unknown, LinkON, LinkOFF };

namespace coralmicro {
namespace arduino {

// Defines a client-side connection to a server using Ethernet.
//
// This is an alias for the ``arduino::SocketClient`` class, where all the
// available functions are defined.
//
// **Example**:
//
// This code enables the Ethernet connection, connects to a server via hostname
// and port, then sends an HTTP GET request and prints the response.
//
// \snippet PoEExamples/examples/EthernetClient/EthernetClient.ino ardu-ethernet-client
using EthernetClient = SocketClient;

// Defines a server using Ethernet.
//
// This is an alias for the ``arduino::SocketServer`` class, where all the
// available functions are defined.
//
// **Example**:
//
// This code starts a server on the board and, when a client connects to it,
// it prints all data read from the client to the board serial console.
//
// \snippet PoEExamples/examples/EthernetServer/EthernetServer.ino ardu-ethernet-server
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

  // Initializes an Ethernet connection, using the specified IP address.
  // The DNS server will be the IP address with the final octet as 1.
  // The gateway will be the IP address with the final octet as 1.
  // The subnet mask will be 255.255.255.0.
  //
  // @param ip The desired IP address.
  // @returns 1 if Ethernet is brought up successfully.
  int begin(IPAddress ip);

  // Initializes an Ethernet connection, using the specified IP address
  // and DNS server.
  // The gateway will be the IP address with the final octet as 1.
  // The subnet mask will be 255.255.255.0.
  //
  // @param ip The desired IP address.
  // @param dns_server The IP address of a DNS server.
  // @returns 1 if Ethernet is brought up successfully.
  int begin(IPAddress ip, IPAddress dns_server);

  // Initializes an Ethernet connection, using the specified IP address,
  // DNS server, and gateway.
  // The subnet mask will be 255.255.255.0.
  //
  // @param ip The desired IP address.
  // @param dns_server The IP address of a DNS server.
  // @param gateway The IP address of the network gateway.
  // @returns 1 if Ethernet is brought up successfully.
  int begin(IPAddress ip, IPAddress dns_server, IPAddress gateway);

  // Initializes an Ethernet connection, using the specified IP address,
  // DNS server, gateway, and subnet mask.
  //
  // @param ip The desired IP address.
  // @param dns_server The IP address of a DNS server.
  // @param gateway The IP address of the network gateway.
  // @param subnet_mask The subnet mask for the network.
  // @returns 1 if Ethernet is brought up successfully.
  int begin(IPAddress ip, IPAddress dns_server, IPAddress gateway,
            IPAddress subnet_mask);

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

// This is the global `EthernetClass` instance you should use instead of
// creating your own instance.
extern coralmicro::arduino::EthernetClass Ethernet;

#endif  // Ethernet_h
