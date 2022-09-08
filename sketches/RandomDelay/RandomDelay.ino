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

// Shows how to generate a random number and delay execution.

#include "Arduino.h"

int led_pin = PIN_LED_USER;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Random Delay!");

  pinMode(led_pin, OUTPUT);
}

void loop() {
  for (int s = 0; s < 5; s++) {
    randomSeed(s);
    for (int i = 0; i < 5; i++) {
      digitalWrite(led_pin, HIGH);
      long value = random(250, 750);
      Serial.print("random seed: ");
      Serial.print(s);
      Serial.print(" value: ");
      Serial.println(value);

      delay(value);
      digitalWrite(led_pin, LOW);
      delay(1000);
    }
  }
}
