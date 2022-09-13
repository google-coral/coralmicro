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

// Executes an SPI transaction using the SPI pins on the board header.
// You must connect the SDI and SDO pins to each other.

// [start-snippet:ardu-spi]
#include "Arduino.h"
#include "SPI.h"

static uint8_t count = 0;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino SPI Example!");

  SPI.begin();
}

void loop() {
  Serial.println("Executing a SPI transaction.");
  if (SPI.transfer(count) == count) {
    Serial.println("Transaction success!");
  } else {
    Serial.println("Transaction failed!");
  }
  count++;
  count %= 255;

  delay(1000);
}
// [end-snippet:ardu-spi]
