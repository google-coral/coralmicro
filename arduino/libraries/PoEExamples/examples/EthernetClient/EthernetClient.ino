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

//! [ardu-ethernet-client] Start doxygen snippet
#include "Arduino.h"
#include "Ethernet.h"

namespace {
coralmicro::arduino::EthernetClient client;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino EthernetClient!");

  if (!Ethernet.begin()) {
    Serial.println("DHCP failed to get an IP.");
    return;
  }

  if (!client.connect("www.example.com", 80)) {
    Serial.println("Connection failed.");
    return;
  }
  Serial.println("Connection successful!");

  const char* kHttpGet = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
  client.write(reinterpret_cast<const uint8_t*>(kHttpGet), strlen(kHttpGet));
}

void loop() {
  if (client && client.available()) {
    Serial.write(client.read());
  }
}
//! [ardu-ethernet-client] End snippet
