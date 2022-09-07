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

#include "libs/base/led.h"
#include "libs/base/utils.h"
#include "libs/rpc/rpc_http_server.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

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
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e rpc_server
//
// Then trigger an RPC request over USB from a Linux computer:
//    python3 -m pip install -r examples/rpc_server/requirements.txt
//    python3 examples/rpc_server/rpc_client.py

// [start-snippet:rpc-server]
namespace coralmicro {
namespace {

void SerialNumber(struct jsonrpc_request* r) {
  auto serial = GetSerialNumber();
  jsonrpc_return_success(r, "{%Q:%.*Q}", "serial_number", serial.size(),
                         serial.c_str());
}

void Main() {
  printf("RPC Server Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("serial_number", SerialNumber);
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
// [end-snippet:rpc-server]
