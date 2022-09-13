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

// Runs the board as an I2C controller device.
//
// NOTE: Executing this example successfully requires two devices, and the
// devices need headers soldered to facilitate connecting the board I2C lines to
// each other. For usage details, see github.com/google-coral/examples/i2c/

#include <cstdint>
#include <vector>

#include "Arduino.h"
#include "Wire.h"
#include "libs/base/utils.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino I2C Controller Example!");
  Wire.begin();

  constexpr int kTargetAddress = 0x42;
  constexpr int kTransferSize = 16;
  auto serial = coralmicro::GetSerialNumber();
  Serial.println("Writing our serial number to the remote device...");
  Wire.beginTransmission(kTargetAddress);
  Wire.write(serial.c_str());
  Wire.endTransmission();

  auto buffer = std::vector<uint8_t>(kTransferSize, 0);
  Serial.println("Reading back our serial number from the remote device...");
  Wire.requestFrom(kTargetAddress, kTransferSize);
  for (int i = 0; i < kTransferSize; ++i) {
    buffer[i] = Wire.read();
  }
  if (memcmp(buffer.data(), serial.data(), kTransferSize) == 0) {
    Serial.println("Readback of data from target device matches written data!");
  } else {
    Serial.println(
        "Readback of data from target device does not match written data...");
  }
}

void loop() {}
