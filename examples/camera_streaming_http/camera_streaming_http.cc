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
#include <vector>

#include "libs/base/http_server.h"
#include "libs/base/led.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"
#include "libs/camera/camera.h"
#include "libs/libjpeg/jpeg.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#if defined(CAMERA_STREAMING_HTTP_ETHERNET)
#include "libs/base/ethernet.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"
#elif defined(CAMERA_STREAMING_HTTP_WIFI)
#include "libs/base/wifi.h"
#endif  // defined(CAMERA_STREAMING_HTTP_ETHERNET)

// Hosts an RPC server on the Dev Board Micro that streams camera images
// to a connected client app.

namespace coralmicro {
namespace {

constexpr char kIndexFileName[] = "/coral_micro_camera.html";
constexpr char kCameraStreamUrlPrefix[] = "/camera_stream";

HttpServer::Content UriHandler(const char* uri) {
  if (StrEndsWith(uri, "index.shtml") ||
      StrEndsWith(uri, "coral_micro_camera.html")) {
    return std::string(kIndexFileName);
  } else if (StrEndsWith(uri, kCameraStreamUrlPrefix)) {
    // [start-snippet:jpeg]
    std::vector<uint8_t> buf(CameraTask::kWidth * CameraTask::kHeight *
                             CameraFormatBpp(CameraFormat::kRgb));
    auto fmt = CameraFrameFormat{
        CameraFormat::kRgb,       CameraFilterMethod::kBilinear,
        CameraRotation::k0,       CameraTask::kWidth,
        CameraTask::kHeight,
        /*preserve_ratio=*/false, buf.data(),
        /*while_balance=*/true};
    if (!CameraTask::GetSingleton()->GetFrame({fmt})) {
      printf("Unable to get frame from camera\r\n");
      return {};
    }

    std::vector<uint8_t> jpeg;
    JpegCompressRgb(buf.data(), fmt.width, fmt.height, /*quality=*/75, &jpeg);
    // [end-snippet:jpeg]
    return jpeg;
  }
  return {};
}

void Main() {
  printf("Camera HTTP Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);

#if defined(CAMERA_STREAMING_HTTP_ETHERNET)
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
  printf("Serving on: %s\r\n", ethernet_ip->c_str());
#elif defined(CAMERA_STREAMING_HTTP_WIFI)
  if (!WiFiTurnOn(/*default_iface=*/false)) {
    printf("Unable to bring up WiFi...\r\n");
    vTaskSuspend(nullptr);
  }
  if (!WiFiConnect(10)) {
    printf("Unable to connect to WiFi...\r\n");
    vTaskSuspend(nullptr);
  }
  if (auto wifi_ip = WiFiGetIp()) {
    printf("Serving on: %s\r\n", wifi_ip->c_str());
  } else {
    printf("Failed to get Wifi Ip\r\n");
    vTaskSuspend(nullptr);
  }
#else   // USB
  std::string usb_ip;
  if (GetUsbIpAddress(&usb_ip)) {
    printf("Serving on: http://%s\r\n", usb_ip.c_str());
  }
#endif  // defined(CAMERA_STREAMING_HTTP_ETHERNET)

  HttpServer http_server;
  http_server.AddUriHandler(UriHandler);
  UseHttpServer(&http_server);

  vTaskSuspend(nullptr);
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
