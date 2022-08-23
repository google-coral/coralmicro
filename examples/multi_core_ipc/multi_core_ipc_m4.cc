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
#include "libs/base/ipc_m4.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/task.h"

// This runs on the M4 core and receives IPC messages from the M7
// and toggles the user LED based on the messages received.
// This must be started by the M7 program.

// [start-sphinx-snippet:ipc-m4]
namespace coralmicro {
namespace {

void HandleM7Message(const uint8_t data[kIpcMessageBufferDataSize]) {
  const auto* msg = reinterpret_cast<const ExampleAppMessage*>(data);
  if (msg->type == ExampleMessageType::kLedStatus) {
    printf("[M4] LED_STATUS received: %s\r\n", msg->led_status ? "ON" : "OFF");
    LedSet(Led::kUser, msg->led_status);

    IpcMessage ack_msg{};
    ack_msg.type = IpcMessageType::kApp;
    auto* app_msg = reinterpret_cast<ExampleAppMessage*>(&ack_msg.message.data);
    app_msg->type = ExampleMessageType::kAck;
    IpcM4::GetSingleton()->SendMessage(ack_msg);
  }
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  printf("M4 started.\r\n");

  coralmicro::IpcM4::GetSingleton()->RegisterAppMessageHandler(
      coralmicro::HandleM7Message);
  vTaskSuspend(nullptr);
}
// [end-sphinx-snippet:ipc-m4]
