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

#include "libs/base/filesystem.h"
#include "libs/base/http_server.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"
#include "libs/camera/camera.h"
#include "libs/libjpeg/jpeg.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Hosts an HTTP server on the Dev Board Micro that serves a camera streaming
// webpage.

namespace coralmicro {

coralmicro::HttpServer g_http_server;
constexpr char kIndexFileName[] = "/coral_micro_camera.html";
constexpr char kCameraStreamUrlPrefix[] = "/camera_stream";

HttpServer::Content UriHandler(const char* uri) {
  if (StrEndsWith(uri, kIndexFileName)) {
    if (std::vector<uint8_t> page_content;
        LfsReadFile(kIndexFileName, &page_content)) {
      return page_content;
    }
    printf("Unable to read: %s\r\n", kIndexFileName);
    return {};
  } else if (StrEndsWith(uri, kCameraStreamUrlPrefix)) {
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
    auto jpeg =
        JpegCompressRgb(buf.data(), fmt.width, fmt.height, /*quality=*/75);
    return std::vector<uint8_t>{jpeg.data, jpeg.data + jpeg.size};
  }
  return {};
}

void Main() {
  // Starts Camera in streaming mode.
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);

  g_http_server.AddUriHandler(UriHandler);
  UseHttpServer(&g_http_server);

  std::string ip;
  if (utils::GetUsbIpAddress(&ip)) {
    printf("Serving on: http://%s%s\r\n", ip.c_str(), kIndexFileName);
  }
}

}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
