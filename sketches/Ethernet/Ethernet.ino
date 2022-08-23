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

#include "Arduino.h"
#include "Ethernet.h"

void printMacAddress(uint8_t* mac) {
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[5], HEX);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Ethernet initialization example!");

  if (!Ethernet.begin()) {
    Serial.println("DHCP failed to get an IP.");
    return;
  }
  Serial.print("Our IP address is: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Our subnet mask is: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Our gateway is: ");
  Serial.println(Ethernet.gatewayIP());

  Serial.print("Our DNS server is: ");
  Serial.println(Ethernet.dnsServerIP());

  Serial.print("Our MAC address is: ");
  uint8_t mac_address[6];
  Ethernet.MACAddress(mac_address);
  printMacAddress(mac_address);
  Serial.println();
}

void loop() {}
