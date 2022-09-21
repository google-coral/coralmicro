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

// Captures an image when the User button is pressed, and serves it over the
// 'get_captured_image' RPC call to a connected Python client. Note: the RPC
// call will fail if the image was never captured.
//
// NOTE: The Python client app works with Windows and Linux only.
// The Python client is available in github.com/google-coral/coralmicro/examples
//
// After uploading the sketch, press the User button to capture an image, and
// run this Python client to fetch the image over USB:
//    python3 -m pip install -r examples/camera_triggered/requirements.txt
//    python3 examples/camera_triggered/camera_triggered_client.py

#include <Arduino.h>
#include <coralmicro_SD.h>
#include <coralmicro_camera.h>
#include <libs/libjpeg/jpeg.h>

#include <cstdint>
#include <memory>

#include "libs/base/utils.h"
#include "libs/rpc/rpc_http_server.h"

using namespace coralmicro::arduino;

int img_counter = 0;
int width = 324;
int height = 324;
int button_pin = PIN_BTN;
int last_button_state = LOW;
int current_button_state = HIGH;
unsigned long last_debounce_time = 0;
constexpr unsigned long kDebounceDelay = 50;

FrameBuffer frame_buffer;

void GetCapturedImage(struct jsonrpc_request* r) {
  if (!frame_buffer.isAllocated()) {
    jsonrpc_return_error(r, -1, "Failed to get image from camera.", nullptr);
  }
  jsonrpc_return_success(
      r, "{%Q: %d, %Q: %d, %Q: %V}", "width", width, "height", height,
      "base64_data", frame_buffer.getBufferSize(), frame_buffer.getBuffer());
}

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera Triggered!");

  pinMode(button_pin, INPUT);

  if (Camera.begin(width, height) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }

  // Set up an RPC server that serves the latest image.
  jsonrpc_export("get_captured_image", GetCapturedImage);
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
}

void loop() {
  int reading = digitalRead(button_pin);
  if (reading != last_button_state) {
    last_debounce_time = millis();
  }
  if ((millis() - last_debounce_time) > kDebounceDelay) {
    if (reading != current_button_state) {
      current_button_state = reading;
      if (current_button_state == HIGH) {
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

        SDFile image_file = SD.open(image_name, FILE_WRITE);
        image_file.write(jpeg.data(), jpeg.size());
        image_file.close();
        img_counter++;
      }
    }
  }
  last_button_state = reading;
}
