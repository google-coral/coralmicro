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
#include "WiFi.h"

void printEncryptionType(uint8_t encryptionType) {
  switch (encryptionType) {
    case ENC_TYPE_WEP:
      Serial.print("WEP");
      return;
    case ENC_TYPE_TKIP:
      Serial.print("TKIP");
      return;
    case ENC_TYPE_CCMP:
      Serial.print("CCMP");
      return;
    case ENC_TYPE_NONE:
      Serial.print("NONE");
      return;
    case ENC_TYPE_AUTO:
      Serial.print("AUTO");
      return;
    case ENC_TYPE_UNKNOWN:
    default:
      Serial.print("UNKNOWN");
      return;
  }
}

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Wi-Fi Scan!");

  int networks = WiFi.scanNetworks();
  Serial.print("Number of networks scanned: ");
  Serial.println(networks);
  for (int i = 0; i < networks; ++i) {
    auto rssi = WiFi.RSSI(i);
    auto ssid = WiFi.SSID(i);
    auto encryption = WiFi.encryptionType(i);
    Serial.print("SSID: ");
    Serial.print(ssid);
    Serial.print(" RSSI: ");
    Serial.print(rssi);
    Serial.print(" Encryption: ");
    printEncryptionType(encryption);
    Serial.println();
  }
}

void loop() {}
