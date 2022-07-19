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

extern "C" void app_main(void* param) {
  // Create and register message handler.
  auto message_handler =
      [](const uint8_t data[coralmicro::kIpcMessageBufferDataSize]) {
        const auto* msg =
            reinterpret_cast<const mp_example::ExampleAppMessage*>(data);
        if (msg->type == mp_example::ExampleMessageType::LED_STATUS) {
          printf("[M4] LED_STATUS received\r\n");
          switch (msg->led_status) {
            case mp_example::LEDStatus::ON: {
              coralmicro::LedSet(coralmicro::Led::kUser, true);
              break;
            }
            case mp_example::LEDStatus::OFF: {
              coralmicro::LedSet(coralmicro::Led::kUser, false);
              break;
            }
            default: {
              printf("Unknown LED_STATUS\r\n");
            }
          }
          coralmicro::IpcMessage reply{};
          reply.type = coralmicro::IpcMessageType::kApp;
          auto* ack = reinterpret_cast<mp_example::ExampleAppMessage*>(
              &reply.message.data);
          ack->type = mp_example::ExampleMessageType::ACKNOWLEDGED;
          coralmicro::IpcM4::GetSingleton()->SendMessage(reply);
        }
      };
  coralmicro::IpcM4::GetSingleton()->RegisterAppMessageHandler(message_handler);
  vTaskSuspend(nullptr);
}
