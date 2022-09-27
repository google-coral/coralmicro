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

#include <Arduino.h>

// Toggles each available GPIO on the two 12-pin headers.
//
// These GPIOs are not powerful enough to drive an LED,
// but you can observe the changes with a multimeter.

constexpr int kGpiosToTest[] = {
    D0, D1, D2, D3,
};

bool high = true;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  Serial.begin(115200);
  Serial.println("Arduino GPIO Example!");

  for (auto pin : kGpiosToTest) {
    pinMode(pin, OUTPUT);
  }

  Serial.println("Periodically toggling header GPIOs...");
}

void loop() {
  for (auto pin : kGpiosToTest) {
    digitalWrite(pin, high ? HIGH : LOW);
  }
  high = !high;
  delay(1000);
}