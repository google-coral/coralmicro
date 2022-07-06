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

#include "libs/base/ipc_m4.h"

#include <cstdio>

#include "libs/base/console_m4.h"
#include "libs/base/message_buffer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"

namespace coralmicro {

void IPCM4::HandleSystemMessage(const ipc::SystemMessage& message) {
    switch (message.type) {
        case ipc::SystemMessageType::kConsoleBufferPtr:
            SetM4ConsoleBufferPtr(static_cast<ipc::StreamBuffer*>(
                message.message.console_buffer_ptr));
            break;
        default:
            printf("Unhandled system message type: %d\r\n",
                   static_cast<int>(message.type));
    }
}

void IPCM4::RxTaskFn() {
    size_t rx_bytes =
        xMessageBufferReceive(rx_queue_->message_buffer, &tx_queue_,
                              sizeof(tx_queue_), portMAX_DELAY);
    if (!rx_bytes) {
        vTaskSuspend(nullptr);
    }
    vTaskResume(tx_task_);

    IPC::RxTaskFn();
}

void IPCM4::Init() {
    uint32_t startup_data;
    mcmgr_status_t status;
    do {
        status = MCMGR_GetStartupData(&startup_data);
    } while (status != kStatus_MCMGR_Success);
    rx_queue_ = reinterpret_cast<ipc::MessageBuffer*>(startup_data);

    IPC::Init();
    vTaskResume(rx_task_);

    // Let the primary core know we're alive.
    MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, 1);
}

}  // namespace coralmicro
