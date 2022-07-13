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
#include "libs/base/ipc_m7.h"
#include "third_party/freertos_kernel/include/task.h"

// This runs on the M7 core and sends IPC messages to the M4 with
// instructions to toggle the user LED.

extern "C" [[noreturn]] void app_main(void* param) {
    // Create and register message handler for the M7.
    auto message_handler =
        [](const uint8_t data[coralmicro::ipc::kMessageBufferDataSize],
           void* param) {
            const auto* msg =
                reinterpret_cast<const mp_example::ExampleAppMessage*>(data);
            if (msg->type == mp_example::ExampleMessageType::ACKNOWLEDGED) {
                printf("[M7] ACK received from M4\r\n");
            }
        };
    coralmicro::IPCM7::GetSingleton()->RegisterAppMessageHandler(
        message_handler, nullptr);
    coralmicro::IPCM7::GetSingleton()->StartM4();

    bool led_status{false};
    auto ipc = coralmicro::IPCM7::GetSingleton();
    while (true) {
        led_status = !led_status;
        printf("---\r\n[M7] Sending M4 LEDStatus::%s\r\n",
               led_status ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(1000));
        coralmicro::ipc::Message msg{};
        msg.type = coralmicro::ipc::MessageType::kApp;
        auto* app_message =
            reinterpret_cast<mp_example::ExampleAppMessage*>(&msg.message.data);
        app_message->type = mp_example::ExampleMessageType::LED_STATUS;
        app_message->led_status =
            led_status ? mp_example::LEDStatus::ON : mp_example::LEDStatus::OFF;
        ipc->SendMessage(msg);
    }
}
