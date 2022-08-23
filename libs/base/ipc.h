/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_BASE_IPC_H_
#define LIBS_BASE_IPC_H_

#include <functional>

#include "libs/base/ipc_message_buffer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {

// Do not instantiate this class.
// It provides shared IPC functions for `IpcM7` and `IpcM4`.
class Ipc {
 public:
  // The function type to handle incoming IPC messages,
  // which must be given to `RegisterAppMessageHandler()` via `IpcM7` or
  // `IpcM4`, respective of which core is receiving the messages.
  //
  // The function receives the IPC message as a byte array of size
  // `kIpcMessageBufferDataSize` (127).
  using AppMessageHandler =
      std::function<void(const uint8_t data[kIpcMessageBufferDataSize])>;
  // @cond Do not generate docs
  virtual void Init();
  // @endcond

  // Sends an IPC message to the other core.
  //
  // @param message The message to send.
  void SendMessage(const IpcMessage& message);

  // Sets a callback function to process incoming IPC messages.
  //
  // @param handler The function to receive incoming messages.
  void RegisterAppMessageHandler(AppMessageHandler handler) {
    app_handler_ = handler;
  }

 private:
  static void StaticFreeRtosMessageEventHandler(uint16_t eventData,
                                                void* context) {
    static_cast<Ipc*>(context)->FreeRtosMessageEventHandler(eventData);
  }

  void FreeRtosMessageEventHandler(uint16_t eventData);

  static void StaticTxTaskFn(void* param) {
    static_cast<Ipc*>(param)->TxTaskFn();
  }

  static void StaticRxTaskFn(void* param) {
    static_cast<Ipc*>(param)->RxTaskFn();
  }

  AppMessageHandler app_handler_ = nullptr;
  constexpr static int kSendMessageNotification = 1;

 protected:
  void HandleAppMessage(const uint8_t data[kIpcMessageBufferDataSize]) {
    if (app_handler_) app_handler_(data);
  }
  virtual void HandleSystemMessage(const IpcSystemMessage& message) = 0;
  virtual void TxTaskFn();
  virtual void RxTaskFn();
  SemaphoreHandle_t tx_semaphore_;
  TaskHandle_t tx_task_, rx_task_;
  IpcMessageBuffer *tx_queue_, *rx_queue_;
};

}  // namespace coralmicro

#endif  // LIBS_BASE_IPC_H_
