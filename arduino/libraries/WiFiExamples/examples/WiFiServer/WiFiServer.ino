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

// Simple example showing the usage of `WiFiServer`.
// After flashing this to the board, you can connect to the
// server on port 31337 via the Wi-Fi connection, and your input
// will be echoed on the server console.
//
// For example, you can connect to the server socket from a new terminal on
// your computer with a utility such as netcat, passing the ip address and port:
//   nc 192.168.247.119 31337
// Then type into the terminal and it will be repeated in the board's serial
// console (also in the Arduino IDE Serial Monitor)

//! [ardu-wifi-server] Start doxygen snippet
#include "Arduino.h"
#include "WiFi.h"

namespace {
const char kSsid[] = "great-access-point";
const char kPsk[] = "letmein";
coralmicro::arduino::WiFiClient client;
coralmicro::arduino::WiFiServer server(31337);
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino WiFiServer!");

  int connected = WL_DISCONNECTED;
  if (strlen(kPsk) > 0) {
    connected = WiFi.begin(kSsid, kPsk);
  } else {
    connected = WiFi.begin(kSsid);
  }
  if (connected == WL_CONNECTED) {
    Serial.print("Connected to network ");
    Serial.println(WiFi.SSID());
  } else {
    Serial.println("Failed to connect to network.");
    return;
  }

  IPAddress ip = WiFi.localIP();

  server.begin();
  Serial.print("Our IP address is ");
  Serial.println(ip);
  Serial.println("Server ready on port 31337");
  // Blocks until a client is connected.
  client = server.available();
}

void loop() {
  // If a client is connected and has data available to read,
  // write the data to the serial console.
  if (client && client.available()) {
    Serial.write(client.read());
    Serial.flush();
  }
}
//! [ardu-wifi-server] End snippet
