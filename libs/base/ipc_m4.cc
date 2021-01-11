#include "libs/base/console_m4.h"
#include "libs/base/message_buffer.h"
#include "libs/base/tasks_m4.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"
#include <cstdio>

static valiant::MessageBuffer *p2s_message_buffer = nullptr;
static valiant::MessageBuffer *s2p_message_buffer = nullptr;

// An interrupt from the other core will trigger this handler.
// We use this to let the waiting task known that it's got data.
static void FreeRtosMessageEventHandler(uint16_t eventData, void *context) {
    BaseType_t higher_priority_woken = pdFALSE;
    xMessageBufferSendCompletedFromISR(p2s_message_buffer->message_buffer, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

static void ipc_task(void *param) {
    uint32_t startup_data;
    mcmgr_status_t status;
    do {
        status = MCMGR_GetStartupData(&startup_data);
    } while (status != kStatus_MCMGR_Success);

    // The primary core sends us the P->S message queue in the startup data.
    p2s_message_buffer = reinterpret_cast<valiant::MessageBuffer*>(startup_data);
    MCMGR_RegisterEvent(kMCMGR_FreeRtosMessageBuffersEvent, FreeRtosMessageEventHandler, 0);

    // Let the primary core know we're alive.
    MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, 1);

    // Sit back and wait until the primary core sends us the address of the S->P queue.
    size_t rx_bytes;
    rx_bytes = xMessageBufferReceive(p2s_message_buffer->message_buffer, &s2p_message_buffer, sizeof(s2p_message_buffer), portMAX_DELAY);
    if (!rx_bytes) {
        printf("Failed to receive message buffer pointer!\r\n");
        while (true);
    }

    valiant::StreamBuffer *console_buffer;
    rx_bytes = xMessageBufferReceive(p2s_message_buffer->message_buffer, &console_buffer, sizeof(console_buffer), portMAX_DELAY);
    if (!rx_bytes) {
        printf("Failed to receive console buffer pointer!\r\n");
        while (true);
    }
    valiant::SetM4ConsoleBufferPtr(console_buffer);

    while (true) {
        taskYIELD();
    }
}

namespace valiant {

void IPCInit() {
    xTaskCreate(ipc_task, "ipc_task", configMINIMAL_STACK_SIZE * 10, NULL, IPC_TASK_PRIORITY, NULL);
}

}  // namespace valiant
