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

#include "examples/multi_core_ipc/example_message.h"
#include "libs/base/check.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/task.h"

// This runs on the M7 core and sends IPC messages to the M4 with
// instructions to toggle the user LED.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e multi_core_ipc
//
// This also flashes the M4 program; the M7 program starts the M4.

// [start-sphinx-snippet:ipc-m7]
namespace coralmicro {
namespace {

void HandleM4Message(const uint8_t data[kIpcMessageBufferDataSize]) {
  const auto* msg = reinterpret_cast<const ExampleAppMessage*>(data);
  if (msg->type == ExampleMessageType::kAck) {
    printf("[M7] ACK received from M4\r\n");
  }
}

[[noreturn]] void Main() {
  printf("Multicore IPC Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  auto* ipc = IpcM7::GetSingleton();
  ipc->RegisterAppMessageHandler(HandleM4Message);
  ipc->StartM4();
  CHECK(ipc->M4IsAlive(500));

  bool led_status = false;
  while (true) {
    led_status = !led_status;
    printf("---\r\n[M7] Sending M4 LED_STATUS: %s\r\n",
           led_status ? "ON" : "OFF");

    IpcMessage msg{};
    msg.type = IpcMessageType::kApp;
    auto* app_msg = reinterpret_cast<ExampleAppMessage*>(&msg.message.data);
    app_msg->type = ExampleMessageType::kLedStatus;
    app_msg->led_status = led_status;
    ipc->SendMessage(msg);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

}  // namespace
}  // namespace coralmicro

extern "C" [[noreturn]] void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
// [end-sphinx-snippet:ipc-m7]
