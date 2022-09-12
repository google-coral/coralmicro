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

// Scan for nearby bluetooth devices and print to console.

#include "Arduino.h"
#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"

constexpr int kMaxNumResults = 10;
constexpr unsigned int kScanPeriodMs = 10000;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Bluetooth Beacon!");

  InitEdgefastBluetooth(nullptr);
  while (!BluetoothReady()) {
    delay(500);
  }
  Serial.println("Bluetooth initialized...");

  if (auto advertised = BluetoothAdvertise(); !advertised) {
    Serial.println("Failed to advertise bluetooth...");
  }
  Serial.println("Advertising bluetooth...");
}

void loop() {
  // Do nothing.
}