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

// Uses the camera hardware's motion interrupt to turn on the User LED
// when motion is detected.

#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"

using namespace coralmicro::arduino;

bool motion_detected = false;

void on_motion(void*) { motion_detected = true; }

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_USER, OUTPUT);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  if (Camera.begin(CameraResolution::CAMERA_R324x324) !=
      CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }
  Camera.setMotionDetectionWindow(0, 0, 320, 320);
  Camera.enableMotionDetection(on_motion);
  Serial.println("Camera Initialed!");
}

void loop() {
  if (motion_detected) {
    Serial.println("Motion Detected!");
    digitalWrite(PIN_LED_USER, HIGH);
    motion_detected = false;
  } else {
    digitalWrite(PIN_LED_USER, LOW);
  }
  delay(100);
}
