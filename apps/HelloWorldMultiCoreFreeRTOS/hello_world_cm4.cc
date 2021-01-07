#include "apps/HelloWorldMultiCoreFreeRTOS/message_buffer.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include <cstdio>
#include <cstring>

static valiant::MessageBuffer *p2s_message_buffer = nullptr;
static valiant::MessageBuffer *s2p_message_buffer = nullptr;

// An interrupt from the other core will trigger this handler.
// We use this to let the waiting task known that it's got data.
static void FreeRtosMessageEventHandler(uint16_t eventData, void *context) {
    BaseType_t higher_priority_woken = pdFALSE;
    xMessageBufferSendCompletedFromISR(p2s_message_buffer->message_buffer, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

extern "C" void app_main(void *param) {
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

    // Send over a string to the primary core, using the S->P queue.
    size_t tx_bytes;
    const char *fixed_str = "Hello from M4.";
    tx_bytes = xMessageBufferSend(s2p_message_buffer->message_buffer, fixed_str, strlen(fixed_str), portMAX_DELAY);
    if (tx_bytes != strlen(fixed_str)) {
        printf("Failed to send string\r\n");
    }

    while(true);
}
