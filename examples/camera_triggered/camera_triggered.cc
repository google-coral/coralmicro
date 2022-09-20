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

#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Captures an image when the User button is pressed, and serves it over the
// 'get_captured_image' RPC call to a connected Python client. Note: the RPC
// call will fail if the image was never captured.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e camera_triggered
//
// NOTE: The Python client app works with Windows and Linux only.
//
// Then press the User button to capture an image, and run this Python client to
// fetch the image over USB:
//    python3 -m pip install -r examples/camera_triggered/requirements.txt
//    python3 examples/camera_triggered/camera_triggered_client.py

// [start-snippet:camera-trigger]
namespace coralmicro {
namespace {
void GetCapturedImage(struct jsonrpc_request* request) {
  int width;
  int height;

  // The width and height are specified by the RPC client.
  if (!JsonRpcGetIntegerParam(request, "width", &width)) return;
  if (!JsonRpcGetIntegerParam(request, "height", &height)) return;

  auto format = CameraFormat::kRgb;
  std::vector<uint8_t> image(width * height * CameraFormatBpp(format));
  CameraFrameFormat fmt{format,
                        CameraFilterMethod::kBilinear,
                        CameraRotation::k270,
                        width,
                        height,
                        /*preserve_ratio=*/false,
                        /*buffer=*/image.data(),
                        /*white_balance=*/true};
  if (!CameraTask::GetSingleton()->GetFrame({fmt})) {
    jsonrpc_return_error(request, -1, "Failed to get image from camera.",
                         nullptr);
    return;
  }

  jsonrpc_return_success(request, "{%Q: %d, %Q: %d, %Q: %V}", "width", width,
                         "height", height, "base64_data", image.size(),
                         image.data());
}

[[noreturn]] void Main() {
  printf("Camera Triggered Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  // Starting Camera in triggered mode.
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kTrigger);

  // Set up an RPC server that serves the latest image.
  jsonrpc_export("get_captured_image", GetCapturedImage);
  UseHttpServer(new JsonRpcHttpServer);

  // Register callback for the user button.
  printf("Press the user button to take a picture.\r\n");
  GpioConfigureInterrupt(
      Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
      /*debounce_interval_us=*/50 * 1e3);
  while (true) {
    vTaskSuspend(nullptr);
    CameraTask::GetSingleton()->Trigger();
    printf("Picture taken\r\n");
  }
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
// [end-snippet:camera-trigger]
