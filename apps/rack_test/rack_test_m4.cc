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

#include "apps/rack_test/rack_test_ipc.h"
#include "libs/base/ipc_m4.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/modified/coremark/core_portme.h"

namespace {
void HandleAppMessage(
    const uint8_t data[coralmicro::kIpcMessageBufferDataSize]) {
  const RackTestAppMessage* app_message =
      reinterpret_cast<const RackTestAppMessage*>(data);
  switch (app_message->message_type) {
    case RackTestAppMessageType::kXor: {
      coralmicro::IpcMessage reply;
      reply.type = coralmicro::IpcMessageType::kApp;
      RackTestAppMessage* app_reply =
          reinterpret_cast<RackTestAppMessage*>(&reply.message.data);
      app_reply->message_type = RackTestAppMessageType::kXor;
      app_reply->message.xor_value =
          (app_message->message.xor_value ^ 0xFEEDDEED);
      coralmicro::IpcM4::GetSingleton()->SendMessage(reply);
      break;
    }
    case RackTestAppMessageType::kCoreMark: {
      coralmicro::IpcMessage reply;
      reply.type = coralmicro::IpcMessageType::kApp;
      RackTestAppMessage* app_reply =
          reinterpret_cast<RackTestAppMessage*>(&reply.message.data);
      app_reply->message_type = RackTestAppMessageType::kCoreMark;
      // ClearCoreMarkBuffer();
      // coremark_main();
      // const char* results = GetCoreMarkResults();
      RunCoreMark(app_message->message.buffer_ptr);
      // app_reply->message.coremark_results = results;
      coralmicro::IpcM4::GetSingleton()->SendMessage(reply);
      break;
    }

    default:
      printf("Unknown message type\r\n");
  }
}
}  // namespace

extern "C" void app_main(void* param) {
  coralmicro::IpcM4::GetSingleton()->RegisterAppMessageHandler(
      HandleAppMessage);
  vTaskSuspend(nullptr);
}
