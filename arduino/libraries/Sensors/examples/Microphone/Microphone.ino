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

// Prints microphone audio samples to the serial console.

// [start-snippet:ardu-pdm]
#include "Arduino.h"
#include "PDM.h"

volatile int samples_read = 0;
std::vector<int32_t> current_samples;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino PDM!");

  Mic.onReceive(onPDMData);
  Mic.begin();
}

void loop() {
  if (samples_read) {
    samples_read = 0;
    Serial.println(current_samples[0]);
  }
}

void onPDMData() {
  samples_read = Mic.available();

  if (samples_read) {
    current_samples.clear();
    Mic.read(current_samples, samples_read);
  }
}
// [end-snippet:ardu-pdm]
