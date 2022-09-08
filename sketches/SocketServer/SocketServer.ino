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

// Opens a network socket on the board and repeats any data received from
// a connected client to the serial console.
//
// After you upload the sketch, you should be able to connect to the socket
// server on port 31337 when connected to the board via USB.
// (This is not compatible with Mac clients.)
//
// For example, you should be able to connect from a Linux terminal with this:
//   nc 10.10.10.1 31337

#include "Arduino.h"
#include "SocketClient.h"
#include "SocketServer.h"

namespace {

coralmicro::arduino::SocketClient client;
coralmicro::arduino::SocketServer server(31337);

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino SocketServer!");
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  server.begin();
  Serial.println("Server ready on port 31337");
  client = server.available();
}

void loop() {
  if (client && client.available()) {
    Serial.write(client.read());
    Serial.flush();
  }
}
