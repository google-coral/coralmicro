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

// Hosts a web page on the Dev Board Micro that streams camera images.
//
// After you upload the sketch, you should be able to see the camera stream
// at http://10.10.10.1/coral_micro_camera.html when connected to the board
// via USB. (This is not compatible with Mac clients.)
//
// For more details, see the examples/camera_streaming_http/README.

#include <coralmicro_camera.h>

#include "Arduino.h"
#include "libs/base/http_server.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"
#include "libs/libjpeg/jpeg.h"

using namespace coralmicro::arduino;

namespace {
coralmicro::HttpServer http_server;
FrameBuffer frame_buffer;

const int width = 324;
const int height = 324;

constexpr char kIndexFileName[] = "/coral_micro_camera.html";
constexpr char kCameraStreamUrlPrefix[] = "/camera_stream";
}  // namespace

coralmicro::HttpServer::Content UriHandler(const char* uri) {
  Serial.print("Request received for ");
  Serial.println(uri);
  if (coralmicro::StrEndsWith(uri, "index.shtml")) {
    return std::string(kIndexFileName);
  } else if (coralmicro::StrEndsWith(uri, kCameraStreamUrlPrefix)) {
    if (!Camera.grab(frame_buffer) == CameraStatus::SUCCESS) {
      return {};
    }

    std::vector<uint8_t> jpeg;
    coralmicro::JpegCompressRgb(frame_buffer.getBuffer(), width, height,
                                /*quality=*/75, &jpeg);
    return jpeg;
  }
  return {};
}

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino Camera HTTP Example!");

  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  // Start Camera
  if (Camera.begin(width, height) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }

  Serial.println("Starting server...");
  http_server.AddUriHandler(UriHandler);
  coralmicro::UseHttpServer(&http_server);

  std::string ip;
  if (coralmicro::GetUsbIpAddress(&ip)) {
    Serial.print("GO TO:   http://");
    Serial.print(ip.c_str());
  }
  http_server.AddUriHandler(UriHandler);
  UseHttpServer(&http_server);
}

void loop() {}
