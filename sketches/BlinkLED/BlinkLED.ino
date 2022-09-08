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

// Blinks the user LED (green).

#include "Arduino.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino Blink LED!");

  pinMode(PIN_LED_USER, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);

  // Turn on Status LED to show the board is on.
  digitalWrite(PIN_LED_STATUS, HIGH);
}

void loop() {
  digitalWrite(PIN_LED_USER, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_USER, LOW);
  delay(1000);
}
