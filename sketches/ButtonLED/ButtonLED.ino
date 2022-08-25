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

/*
  Turns the user LED (green) on when you press the user button
  on the Coral Dev Board Micro.
*/

// [start-snippet:ardu-led]
#include "Arduino.h"

int ledPin = PIN_LED_USER;
int buttonPin = PIN_BTN;
PinStatus val = LOW;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Button LED!");

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  val = digitalRead(buttonPin);
  digitalWrite(ledPin, val == LOW ? HIGH : LOW);
}
// [end-snippet:ardu-led]
