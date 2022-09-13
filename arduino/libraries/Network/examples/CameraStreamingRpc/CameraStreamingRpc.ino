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

// Hosts an RPC server on the Dev Board Micro that streams camera images
// to a connected Python client app through an RPC server.
//
// NOTE: The Python client app works on Windows and Linux only.
// The Python client is available in github.com/google-coral/coralmicro/examples
//
// After uploading the sketch, run this Python client to see the camera stream:
// python3 examples/camera_streaming_rpc/camera_streaming_app.py --host_ip 10.10.10.1
//
// For more details, see the examples/camera_streaming_rpc/README.

#include <Arduino.h>
#include <coral_micro.h>
#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>
#include <optional>

#include "libs/base/utils.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"

namespace {
using namespace coralmicro;
using namespace coralmicro::arduino;

std::optional<CameraFormat> CheckCameraFormat(const std::string& format) {
  if (format == "RGB") return CameraFormat::kRgb;

  if (format == "GRAY") return CameraFormat::kY8;

  if (format == "RAW") return CameraFormat::kRaw;

  return std::nullopt;
}

std::optional<CameraFilterMethod> CheckCameraFilterMethod(
    const std::string& method) {
  if (method == "BILINEAR") return CameraFilterMethod::kBilinear;

  if (method == "NEAREST_NEIGHBOR") return CameraFilterMethod::kNearestNeighbor;

  return std::nullopt;
}

std::optional<CameraRotation> CheckCameraRotation(int rotation) {
  switch (rotation) {
    case 0:
      return CameraRotation::k0;
    case 90:
      return CameraRotation::k90;
    case 180:
      return CameraRotation::k180;
    case 270:
      return CameraRotation::k270;
    default:
      return std::nullopt;
  }
}

void GetImageFromCamera(struct jsonrpc_request* request) {
  int width;
  if (!JsonRpcGetIntegerParam(request, "width", &width)) return;

  int height;
  if (!JsonRpcGetIntegerParam(request, "height", &height)) return;

  std::string format_rpc;
  if (!JsonRpcGetStringParam(request, "format", &format_rpc)) return;

  auto format = CheckCameraFormat(format_rpc);
  if (!format.has_value()) {
    jsonrpc_return_error(request, -1, "Unknown 'format'", nullptr);
    return;
  }

  std::string filter_rpc;
  if (!JsonRpcGetStringParam(request, "filter", &filter_rpc)) return;

  auto filter = CheckCameraFilterMethod(filter_rpc);
  if (!filter.has_value()) {
    jsonrpc_return_error(request, -1, "Unknown 'filter'", nullptr);
    return;
  }

  int rotation_rpc;
  if (!JsonRpcGetIntegerParam(request, "rotation", &rotation_rpc)) return;

  auto rotation = CheckCameraRotation(rotation_rpc);
  if (!rotation.has_value()) {
    jsonrpc_return_error(request, -1, "Unknown 'rotation'", nullptr);
    return;
  }

  bool awb;
  if (!JsonRpcGetBooleanParam(request, "auto_white_balance", &awb)) return;

  Camera.begin(width, height, *format, *filter, *rotation, true, awb);
  FrameBuffer frame_buffer;
  if (Camera.grab(frame_buffer) != CameraStatus::SUCCESS) {
    jsonrpc_return_error(request, -1, "Failed to get image from camera.",
                         nullptr);
    return;
  }
  jsonrpc_return_success(
      request, "{%Q: %d, %Q: %d, %Q: %V}", "width", width, "height", height,
      "base64_data", frame_buffer.getBufferSize(), frame_buffer.getBuffer());
}
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera Streaming RPC!");

  jsonrpc_export("get_image_from_camera", GetImageFromCamera);
  UseHttpServer(new JsonRpcHttpServer);

  std::string ip;
  if (coralmicro::GetUsbIpAddress(&ip)) {
    Serial.print("Our ip: ");
    Serial.println(ip.c_str());
  }
}

void loop() {}
