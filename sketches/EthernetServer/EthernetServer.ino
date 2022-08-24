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

// Simple example showing the usage of `EthernetServer`.
// After flashing this to the board, you can connect to the
// server on port 31337 via the Ethernet connection, and your input
// will be echoed on the server console.
//
// Ex: `nc 172.16.243.80 31337`

namespace {
coralmicro::arduino::EthernetClient client;
coralmicro::arduino::EthernetServer server(31337);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino EthernetServer!");

  if (!Ethernet.begin()) {
    Serial.println("DHCP failed to get an IP.");
    return;
  }

  if (!client.connect("www.example.com", 80)) {
    Serial.println("Connection failed.");
    return;
  }
  Serial.println("Connection successful!");
  IPAddress ip = Ethernet.localIP();

  server.begin();
  Serial.print("Our IP address is ");
  Serial.println(ip);
  Serial.println("Server ready on port 31337");
  client = server.available();
}

void loop() {
  if (client && client.available()) {
    Serial.write(client.read());
  }
}