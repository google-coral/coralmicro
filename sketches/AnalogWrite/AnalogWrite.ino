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

// Reads analog input from ADC1 channel B (A0; pin 4 on the left-side header)
// and writes it to the PWM_A pin (A3; pin 10 on the left-side header).

#include "Arduino.h"

int pwm_pin = A3;
int analog_pin = A0;
int val = 0;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Analog Write!");
}

void loop() {
  val = analogRead(analog_pin);
  analogWrite(pwm_pin, val / 4);
}
