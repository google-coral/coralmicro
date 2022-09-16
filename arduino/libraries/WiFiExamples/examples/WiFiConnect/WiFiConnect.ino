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

//! [ardu-wifi-connect] Start doxygen snippet
#include "Arduino.h"
#include "WiFi.h"

const char kSsid[] = "great-access-point";
const char kPsk[] = "letmein";

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
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Wi-Fi Connect!");

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

  uint8_t mac[6];
  WiFi.macAddress(mac);
  uint8_t bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("Our MAC address: ");
  printMacAddress(mac);
  Serial.println();
  Serial.print("Network BSSID: ");
  printMacAddress(bssid);
  Serial.println();

  int32_t rssi = WiFi.RSSI();
  Serial.print("Network RSSI: ");
  Serial.println(rssi);

  IPAddress ip = WiFi.localIP();
  Serial.print("Ip address: ");
  Serial.println(ip);

  WiFi.disconnect();
}

void loop() {}
//! [ardu-wifi-connect] End snippet
