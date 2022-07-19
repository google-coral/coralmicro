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

// User button on the main board.
constexpr int Button = PIN_BTN;
unsigned long duration;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting");
    pinMode(Button, INPUT);
}

void loop() {
    duration = pulseIn(Button, HIGH);
    Serial.print("pulseIn duration: ");
    Serial.println(duration);
    duration = pulseInLong(Button, HIGH);
    Serial.print("pulseInLong duration: ");
    Serial.println(duration);
}