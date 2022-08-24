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

// Takes an image and save it in the filesystem when you press the user button
// on the Coral Dev Board Micro.

#include <coralmicro_SD.h>
#include <coralmicro_camera.h>
#include <libs/libjpeg/jpeg.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"

using namespace coralmicro::arduino;

int img_counter = 0;
int width = 324;
int height = 324;
int buttonPin = PIN_BTN;
PinStatus val = LOW;
FrameBuffer frame_buffer;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera Triggered!");

  pinMode(buttonPin, INPUT);

  if (Camera.begin(width, height) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }
}

void loop() {
  pulseIn(buttonPin, HIGH);
  Serial.println("Button triggered, taking image");

  if (!Camera.grab(frame_buffer) == CameraStatus::SUCCESS) {
    return;
  }

  std::vector<uint8_t> jpeg;
  coralmicro::JpegCompressRgb(frame_buffer.getBuffer(), width, height,
                              /*quality=*/75, &jpeg);

  char image_name[64];
  sprintf(image_name, "image_%d.jpeg", img_counter);
  Serial.print("Saving image as: ");
  Serial.println(image_name);

  SDFile imageFile = SD.open(image_name, FILE_WRITE);
  imageFile.write(jpeg.data(), jpeg.size());
  imageFile.close();
  img_counter++;
}
