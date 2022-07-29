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
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// A simple example that captures an image and served over the
// 'get_captured_image' rpc call when the User Button is clicked. Note: the rpc
// call will fail if the image was never captured.

namespace coralmicro {
void GetCapturedImage(struct jsonrpc_request* request) {
  int width;
  if (!JsonRpcGetIntegerParam(request, "width", &width)) return;

  int height;
  if (!JsonRpcGetIntegerParam(request, "height", &height)) return;

  auto format = CameraFormat::kRgb;
  std::vector<uint8_t> image(width * height * CameraFormatBpp(format));
  CameraFrameFormat fmt{format,
                        CameraFilterMethod::kBilinear,
                        CameraRotation::k0,
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
  // Starting Camera in triggered mode.
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kTrigger);

  // Set up an RPC server that serves the latest image.
  jsonrpc_export("get_captured_image", GetCapturedImage);
  UseHttpServer(new JsonRpcHttpServer);

  // Register callback for the user button.
  printf("Press the user button to take a picture.\r\n");
  GpioConfigureInterrupt(
      Gpio::kUserButton,
      GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); });
  while (true) {
    vTaskSuspend(nullptr);
    CameraTask::GetSingleton()->Trigger();
    printf("Picture taken\r\n");
  }
}

}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
