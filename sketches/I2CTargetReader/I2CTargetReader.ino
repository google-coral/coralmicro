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
#include "Wire.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino I2C Target Reader!");

  Wire.begin(0x42);
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);
}

void loop() { delay(100); }

void receiveEvent(int n) {
  while (Wire.available() > 1) {
    char c = Wire.read();
    Serial.print(c);
  }
  int x = Wire.read();
  Serial.println(x);
}
