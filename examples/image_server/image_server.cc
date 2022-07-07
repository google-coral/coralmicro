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

#include "libs/base/check.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"

#if defined(IMAGE_SERVER_ETHERNET)
#include "libs/base/ethernet.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"
#endif  // defined(IMAGE_SERVER_ETHERNET)

#if defined(IMAGE_SERVER_WIFI)
#include "libs/base/wifi.h"
#endif  // defined(IMAGE_SERVER_WIFI)

namespace {
using coralmicro::testlib::JsonRpcGetBooleanParam;
using coralmicro::testlib::JsonRpcGetIntegerParam;
using coralmicro::testlib::JsonRpcGetStringParam;

bool FormatStringToFormat(const std::string& format_string,
                          coralmicro::CameraFormat* format) {
  if (format_string == "RGB") {
    *format = coralmicro::CameraFormat::kRgb;
    return true;
  }
  if (format_string == "GRAY") {
    *format = coralmicro::CameraFormat::kY8;
    return true;
  }
  if (format_string == "RAW") {
    *format = coralmicro::CameraFormat::kRaw;
    return true;
  }
  return false;
}

bool FilterStringToFilter(const std::string& filter_string,
                          coralmicro::CameraFilterMethod* filter) {
  if (filter_string == "BILINEAR") {
    *filter = coralmicro::CameraFilterMethod::kBilinear;
    return true;
  }
  if (filter_string == "NEAREST_NEIGHBOR") {
    *filter = coralmicro::CameraFilterMethod::kNearestNeighbor;
    return true;
  }
  return false;
}

bool RotationIntToRotation(const int& rotation_int,
                           coralmicro::CameraRotation* rotation) {
  switch (rotation_int) {
    case 0:
      *rotation = coralmicro::CameraRotation::k0;
      return true;
    case 90:
      *rotation = coralmicro::CameraRotation::k90;
      return true;
    case 180:
      *rotation = coralmicro::CameraRotation::k180;
      return true;
    case 270:
      *rotation = coralmicro::CameraRotation::k270;
      return true;
    default:
      return false;
  }
}

void get_image_from_camera(struct jsonrpc_request* request) {
  int width = coralmicro::CameraTask::kWidth;
  int height = coralmicro::CameraTask::kHeight;
  auto format = coralmicro::CameraFormat::kRgb;
  auto filter = coralmicro::CameraFilterMethod::kBilinear;
  auto rotation = coralmicro::CameraRotation::k0;
  bool auto_white_balance = true;

  std::string format_rpc, filter_rpc;
  int rotation_rpc;
  bool awb_rpc;
  if (!JsonRpcGetIntegerParam(request, "width", &width)) {
    return;
  }
  if (!JsonRpcGetIntegerParam(request, "height", &height)) {
    return;
  }
  if (!JsonRpcGetStringParam(request, "format", &format_rpc)) {
    return;
  }
  if (!JsonRpcGetStringParam(request, "filter", &filter_rpc)) {
    return;
  }
  if (!JsonRpcGetIntegerParam(request, "rotation", &rotation_rpc)) {
    return;
  }
  if (!JsonRpcGetBooleanParam(request, "auto_white_balance", &awb_rpc)) {
    return;
  }

  if (!FormatStringToFormat(format_rpc, &format)) {
    jsonrpc_return_error(request, -1, "Unknown 'format'", nullptr);
    return;
  }
  if (!FilterStringToFilter(filter_rpc, &filter)) {
    jsonrpc_return_error(request, -1, "Unknown 'filter'", nullptr);
    return;
  }
  if (!RotationIntToRotation(rotation_rpc, &rotation)) {
    jsonrpc_return_error(request, -1, "Unknown 'rotation'", nullptr);
    return;
  }

  //! [camera-stream] Doxygen snippet for camera.h
  coralmicro::CameraTask::GetSingleton()->SetPower(true);
  coralmicro::CameraTask::GetSingleton()->Enable(
      coralmicro::CameraMode::kStreaming);
  std::vector<uint8_t> image(width * height *
                             coralmicro::CameraTask::FormatToBPP(format));
  coralmicro::CameraFrameFormat fmt{
      format, filter, rotation,     width,
      height, false,  image.data(), auto_white_balance};
  auto ret = coralmicro::CameraTask::GetSingleton()->GetFrame({fmt});
  coralmicro::CameraTask::GetSingleton()->Disable();
  coralmicro::CameraTask::GetSingleton()->SetPower(false);
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

}  // namespace

extern "C" void app_main(void* param) {
#if defined(IMAGE_SERVER_ETHERNET)
  CHECK(coralmicro::EthernetInit(/*default_iface=*/true));
  auto* ethernet = coralmicro::EthernetGetInterface();
  if (!ethernet) {
    printf("Unable to bring up ethernet...\r\n");
    vTaskSuspend(nullptr);
  }
  auto ethernet_ip = coralmicro::EthernetGetIp();
  if (!ethernet_ip.has_value()) {
    printf("Unable to get Ethernet IP\r\n");
    vTaskSuspend(nullptr);
  }
  printf("Starting Image RPC Server on: %s\r\n", ethernet_ip.value().c_str());
  jsonrpc_init(nullptr, &ethernet_ip.value());
  jsonrpc_export("get_ethernet_ip", [](struct jsonrpc_request* request) {
    jsonrpc_return_success(
        request, "{%Q: %Q}", "ethernet_ip",
        reinterpret_cast<std::string*>(request->ctx->response_cb_data)
            ->c_str());
  });
#elif defined(IMAGE_SERVER_WIFI)
  if (!coralmicro::WiFiTurnOn()) {
    printf("Unable to bring up wifi...\r\n");
  }
  jsonrpc_export(coralmicro::testlib::kMethodWiFiConnect,
                 coralmicro::testlib::WiFiConnect);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiGetIp,
                 coralmicro::testlib::WiFiGetIp);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiGetStatus,
                 coralmicro::testlib::WiFiGetStatus);

#else
  printf("Starting Image RPC Server...\r\n");
  jsonrpc_init(nullptr, nullptr);
#endif  // defined(IMAGE_SERVER_ETHERNET)
  jsonrpc_export("get_image_from_camera", get_image_from_camera);
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
  printf("Server started...\r\n");
  vTaskSuspend(nullptr);
}
