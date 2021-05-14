#include "libs/base/console_m4.h"
#include "libs/base/ipc_m4.h"
#include "libs/base/message_buffer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"
#include <cstdio>

namespace valiant {

IPC* IPC::GetSingleton() {
    static IPCM4 ipc;
    return static_cast<IPC*>(&ipc);
}

void IPCM4::HandleSystemMessage(const ipc::SystemMessage& message) {
    switch (message.type) {
        case ipc::SystemMessageType::CONSOLE_BUFFER_PTR:
            SetM4ConsoleBufferPtr(
                    reinterpret_cast<ipc::StreamBuffer*>(message.message.console_buffer_ptr));
            break;
        default:
            printf("Unhandled system message type: %d\r\n", static_cast<int>(message.type));
    }
}

void IPCM4::RxTaskFn(void *param) {
    size_t rx_bytes;
    rx_bytes = xMessageBufferReceive(rx_queue_->message_buffer, &tx_queue_, sizeof(tx_queue_), portMAX_DELAY);
    if (!rx_bytes) {
        vTaskSuspend(NULL);
    }
    vTaskResume(tx_task_);

    IPC::RxTaskFn(param);
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

}  // namespace valiant
