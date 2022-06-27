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

#ifndef LIBS_BASE_IPC_H_
#define LIBS_BASE_IPC_H_

#include <functional>

#include "libs/base/message_buffer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coral::micro {

class IPC {
   public:
    using AppMessageHandler = std::function<void(
        const uint8_t data[ipc::kMessageBufferDataSize], void*)>;
    virtual void Init();
    void SendMessage(const ipc::Message& message);
    void RegisterAppMessageHandler(AppMessageHandler, void* param);

   private:
    static void StaticFreeRtosMessageEventHandler(uint16_t eventData,
                                                  void* context);
    void FreeRtosMessageEventHandler(uint16_t eventData);
    static void StaticTxTaskFn(void* param) {
        static_cast<IPC*>(param)->TxTaskFn();
    }

    static void StaticRxTaskFn(void* param) {
        static_cast<IPC*>(param)->RxTaskFn();
    }

    AppMessageHandler app_handler_ = nullptr;
    constexpr static int kSendMessageNotification = 1;
    void* app_handler_param_ = nullptr;

   protected:
    void HandleAppMessage(const uint8_t data[ipc::kMessageBufferDataSize]);
    virtual void HandleSystemMessage(const ipc::SystemMessage& message) = 0;
    virtual void TxTaskFn();
    virtual void RxTaskFn();
    SemaphoreHandle_t tx_semaphore_;
    TaskHandle_t tx_task_, rx_task_;
    ipc::MessageBuffer *tx_queue_, *rx_queue_;
};

}  // namespace coral::micro

#endif  // LIBS_BASE_IPC_H_
