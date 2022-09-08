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

// Shifts bytes, using a GPIO pin as the bit output and another GPIO as clock.

#include "Arduino.h"

// GPIO Pin 7 on the left-side header.
constexpr uint8_t data_pin = D2;
// GPIO Pin 8 on the left-side header.
constexpr uint8_t clock_pin = D1;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Shift!");

  pinMode(data_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
}

void loop() {
  for (uint8_t val = 0; val < 256; val++) {
    shiftOut(data_pin, clock_pin, LSBFIRST, val);
    delay(1000);
  }
}
