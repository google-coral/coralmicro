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

// Detects User button presses and prints the press duration as pulses.

#include <Arduino.h>

// User button on the main board.
constexpr int button_pin = PIN_BTN;
unsigned long duration;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino pulseIn!");

  Serial.println("Starting");
  pinMode(button_pin, INPUT);
}

void loop() {
  duration = pulseIn(button_pin, HIGH);
  Serial.print("pulseIn duration: ");
  Serial.println(duration);
  duration = pulseInLong(button_pin, HIGH);
  Serial.print("pulseInLong duration: ");
  Serial.println(duration);
}
