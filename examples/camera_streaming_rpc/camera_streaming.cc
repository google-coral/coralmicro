// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>
#include <optional>

#include "libs/base/led.h"
#include "libs/base/utils.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"

#if defined(CAMERA_STREAMING_ETHERNET)
#include "libs/base/ethernet.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"
#endif  // defined(CAMERA_STREAMING_ETHERNET)

#if defined(CAMERA_STREAMING_WIFI)
#include "libs/base/wifi.h"
#endif  // defined(CAMERA_STREAMING_WIFI)

// Hosts a web page on the Dev Board Micro that streams camera images.

namespace coralmicro {
namespace {

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

  //! [camera-stream] Doxygen snippet for camera.h
  std::vector<uint8_t> image(width * height * CameraFormatBpp(*format));
  CameraFrameFormat fmt{*format, *filter, *rotation,    width,
                        height,  false,   image.data(), awb};
  auto ret = CameraTask::GetSingleton()->GetFrame({fmt});
  //! [camera-stream] End snippet

  if (!ret) {
    jsonrpc_return_error(request, -1, "Failed to get image from camera.",
                         nullptr);
    return;
  }
  jsonrpc_return_success(request, "{%Q: %d, %Q: %d, %Q: %V}", "width", width,
                         "height", height, "base64_data", image.size(),
                         image.data());
}

void Main() {
  printf("Camera Streaming RPC Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

#if defined(CAMERA_STREAMING_ETHERNET)
  EthernetInit(/*default_iface=*/false);
  auto* ethernet = EthernetGetInterface();
  if (!ethernet) {
    printf("Unable to bring up ethernet...\r\n");
    vTaskSuspend(nullptr);
  }

  auto ethernet_ip = EthernetGetIp();
  if (!ethernet_ip.has_value()) {
    printf("Unable to get Ethernet IP\r\n");
    vTaskSuspend(nullptr);
  }

  printf("Starting Image RPC Server on: %s\r\n", ethernet_ip->c_str());
  jsonrpc_init(nullptr, &ethernet_ip.value());
  jsonrpc_export(
      "get_ethernet_ip", +[](struct jsonrpc_request* request) {
        jsonrpc_return_success(
            request, "{%Q: %Q}", "ethernet_ip",
            static_cast<std::string*>(request->ctx->response_cb_data)->c_str());
      });
#elif defined(CAMERA_STREAMING_WIFI)
  if (!WiFiTurnOn(/*default_iface=*/false)) {
    printf("Unable to bring up WiFi...\r\n");
    vTaskSuspend(nullptr);
  }
  if (!WiFiConnect(10)) {
    printf("Unable to connect to WiFi...\r\n");
    vTaskSuspend(nullptr);
  }
  if (auto wifi_ip = WiFiGetIp()) {
    printf("Starting Image RPC Server on: %s\r\n", wifi_ip->c_str());
    jsonrpc_init(nullptr, &wifi_ip.value());
    jsonrpc_export(
        "wifi_get_ip", +[](struct jsonrpc_request* request) {
          jsonrpc_return_success(
              request, "{%Q: %Q}", "wifi_ip",
              static_cast<std::string*>(request->ctx->response_cb_data)
                  ->c_str());
        });
  } else {
    printf("Failed to get Wifi Ip\r\n");
    vTaskSuspend(nullptr);
  }
#else
  std::string usb_ip;
  if (!GetUsbIpAddress(&usb_ip)) {
    printf("Failed to get USB's Ip Address\r\n");
    vTaskSuspend(nullptr);
  }
  printf("Starting Image RPC Server on: %s\r\n", usb_ip.c_str());
  jsonrpc_init(nullptr, nullptr);
#endif  // defined(CAMERA_STREAMING_ETHERNET)

  // Turn on camera for streaming mode.
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);

  jsonrpc_export("get_image_from_camera", GetImageFromCamera);
  UseHttpServer(new JsonRpcHttpServer);
  vTaskSuspend(nullptr);
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
