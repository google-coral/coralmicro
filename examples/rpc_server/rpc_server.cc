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
#include <memory>
#include <vector>

#include "libs/base/led.h"
#include "libs/base/utils.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
// Runs a local server with two RPC endpoints: 'serial_number', which
// returns the board's SN, and 'take_picture', which captures an image with
// the board's camera and return it via JSON.
//
// The 'take_picture' response looks like this:
//
// {
// 'id': int,
// 'result':
//     {
//     'width': int,
//     'height': int,
//     'base64_data': image_bytes
//     }
// }

namespace coralmicro {
namespace {

void SerialNumber(struct jsonrpc_request* r) {
  auto serial = utils::GetSerialNumber();
  jsonrpc_return_success(r, "{%Q:%.*Q}", "serial_number", serial.size(),
                         serial.c_str());
}
void TakePicture(struct jsonrpc_request* r) {
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);
  CameraTask::GetSingleton()->DiscardFrames(1);
  std::vector<uint8_t> image_buffer(CameraTask::kWidth * CameraTask::kHeight *
                                    3);
  CameraFrameFormat fmt;
  fmt.fmt = CameraFormat::kRgb;
  fmt.filter = CameraFilterMethod::kBilinear;
  fmt.width = CameraTask::kWidth;
  fmt.height = CameraTask::kHeight;
  fmt.preserve_ratio = false;
  fmt.buffer = image_buffer.data();
  bool ret = CameraTask::GetSingleton()->GetFrame({fmt});
  CameraTask::GetSingleton()->Disable();
  CameraTask::GetSingleton()->SetPower(false);
  if (ret) {
    jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V}", "width",
                           CameraTask::kWidth, "height", CameraTask::kHeight,
                           "base64_data", image_buffer.size(),
                           image_buffer.data());
  } else {
    jsonrpc_return_error(r, -1, "failure", nullptr);
  }
}

void Main() {
  printf("Coral Micro RPC Server Example!\r\n");
  // Status LED turn on to shows board is on.
  LedSet(Led::kStatus, true);

  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("serial_number", SerialNumber);
  jsonrpc_export("take_picture", TakePicture);
  UseHttpServer(new JsonRpcHttpServer);
  printf("RPC server ready\r\n");
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
