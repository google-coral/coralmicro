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

#include "libs/base/ipc.h"

#include "libs/base/check.h"
#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"

namespace coralmicro {

extern "C" uint32_t __RPMSG_SH_MEM_START[];

void Ipc::FreeRtosMessageEventHandler(uint16_t eventData) {
  BaseType_t higher_priority_woken = pdFALSE;
  xStreamBufferSendCompletedFromISR(
      reinterpret_cast<StreamBufferHandle_t>(
          reinterpret_cast<uint32_t>(__RPMSG_SH_MEM_START) | eventData),
      &higher_priority_woken);
  portYIELD_FROM_ISR(higher_priority_woken);
}

void Ipc::SendMessage(const IpcMessage& message) {
  if (!tx_task_ || !tx_semaphore_) {
    return;
  }
  while (xTaskNotifyIndexed(tx_task_, kSendMessageNotification,
                            reinterpret_cast<uint32_t>(&message),
                            eSetValueWithoutOverwrite) == pdFALSE) {
    taskYIELD();
  }
  CHECK(xSemaphoreTake(tx_semaphore_, portMAX_DELAY) == pdTRUE);
}

void Ipc::TxTaskFn() {
  while (true) {
    IpcMessage* message;
    xTaskNotifyWaitIndexed(kSendMessageNotification, 0, 0,
                           reinterpret_cast<uint32_t*>(&message),
                           portMAX_DELAY);
    xMessageBufferSend(tx_queue_->message_buffer, message, sizeof(*message),
                       portMAX_DELAY);
    CHECK(xSemaphoreGive(tx_semaphore_) == pdTRUE);
  }
}

void Ipc::RxTaskFn() {
  while (true) {
    IpcMessage rx_message;
    size_t rx_bytes =
        xMessageBufferReceive(rx_queue_->message_buffer, &rx_message,
                              sizeof(rx_message), portMAX_DELAY);
    if (rx_bytes == 0) continue;

    switch (rx_message.type) {
      case IpcMessageType::kSystem:
        HandleSystemMessage(rx_message.message.system);
        break;
      case IpcMessageType::kApp:
        HandleAppMessage(rx_message.message.data);
        break;
      default:
        printf("Unhandled IPC message type %d\r\n",
               static_cast<int>(rx_message.type));
        break;
    }
  }
}

void Ipc::Init() {
  tx_semaphore_ = xSemaphoreCreateBinary();
  CHECK(tx_semaphore_);
  MCMGR_RegisterEvent(kMCMGR_FreeRtosMessageBuffersEvent,
                      StaticFreeRtosMessageEventHandler, this);
  CHECK(xTaskCreate(Ipc::StaticTxTaskFn, "ipc_tx_task",
                    configMINIMAL_STACK_SIZE * 10, this, kIpcTaskPriority,
                    &tx_task_) == pdPASS);
  CHECK(xTaskCreate(Ipc::StaticRxTaskFn, "ipc_rx_task",
                    configMINIMAL_STACK_SIZE * 10, this, kIpcTaskPriority,
                    &rx_task_) == pdPASS);
  vTaskSuspend(tx_task_);
  vTaskSuspend(rx_task_);
}

}  // namespace coralmicro
