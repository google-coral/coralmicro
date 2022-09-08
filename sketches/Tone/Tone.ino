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

// Runs simple tone cycles with the PWM pins on the board header.

#include <Arduino.h>

// PWM_A on the left-side header
constexpr int pin_10 = A3;
// PWM_B on the left-side header
constexpr int pin_9 = A4;
// Note: These pins output a max of 1.8V

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Tone!");
}

void loop() {
  tone(/*pin=*/pin_10, /*frequency=*/1000);
  delay(1000);
  noTone(pin_10);
  tone(/*pin=*/pin_9, /*frequency=*/2000);
  delay(1000);
  noTone(pin_9);
}
