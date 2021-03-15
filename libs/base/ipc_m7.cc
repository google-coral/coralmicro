#include "libs/base/console_m7.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/message_buffer.h"
#include "libs/base/tasks_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"
#include <cstdio>
#include <cstring>

#define CORE1_BOOT_ADDRESS 0x20200000
unsigned int m4_binary_start __attribute__((weak)) = 0xdeadbeef;
unsigned int m4_binary_end __attribute__((weak)) = 0xdeadbeef;
unsigned int m4_binary_size __attribute__((weak)) = 0xdeadbeef;

// Define two message buffers.
// One for Primary->Secondary, and one for Secondary->Primary.
constexpr size_t kP2SBufferBytes = 1024;
static valiant::MessageBuffer *p2s_message_buffer = nullptr;
static uint8_t p2s_message_buffer_storage[sizeof(valiant::MessageBuffer) + kP2SBufferBytes] __attribute__((section(".noinit.$rpmsg_sh_mem")));

constexpr size_t kS2PBufferBytes = 1024;
static valiant::MessageBuffer *s2p_message_buffer = nullptr;
static uint8_t s2p_message_buffer_storage[sizeof(valiant::MessageBuffer) + kS2PBufferBytes] __attribute__((section(".noinit.$rpmsg_sh_mem")));

// This will be set to 1 when we receive an event from the secondary core.
static uint16_t remote_alive = 0;

static void RemoteAppEventHandler(uint16_t eventData, void *context) {
    remote_alive = 1;
}

// An interrupt from the other core will trigger this handler.
// We use this to let the waiting task known that it's got data.
static void FreeRtosMessageEventHandler(uint16_t eventData, void *context) {
    BaseType_t higher_priority_woken = pdFALSE;
    xMessageBufferSendCompletedFromISR(s2p_message_buffer->message_buffer, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

char buf[256];
void ipc_task(void *param) {
    p2s_message_buffer = reinterpret_cast<valiant::MessageBuffer*>(p2s_message_buffer_storage);
    p2s_message_buffer->message_buffer = xMessageBufferCreateStatic(kP2SBufferBytes, p2s_message_buffer->message_buffer_storage, &p2s_message_buffer->static_message_buffer);
    if (!p2s_message_buffer->message_buffer) {
        printf("Failed to allocate message buffer!\r\n");
        while (true);
    }
    p2s_message_buffer->len = kP2SBufferBytes;

    s2p_message_buffer = reinterpret_cast<valiant::MessageBuffer*>(s2p_message_buffer_storage);
    s2p_message_buffer->message_buffer = xMessageBufferCreateStatic(kS2PBufferBytes, s2p_message_buffer->message_buffer_storage, &s2p_message_buffer->static_message_buffer);
    if (!s2p_message_buffer->message_buffer) {
        printf("Failed to allocate message buffer!\r\n");
        while (true);
    }
    s2p_message_buffer->len = kS2PBufferBytes;

    // Load the remote core's memory space with the program binary.
    uint32_t m4_start = (uint32_t)&m4_binary_start;
    uint32_t m4_size = (uint32_t)&m4_binary_size;
    printf("CM4 binary is %lu bytes at 0x%lx\r\n", m4_size, m4_start);
    memcpy((void*)CORE1_BOOT_ADDRESS, (void*)m4_start, m4_size);

    // Register callbacks for communication with the other core.
    MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, RemoteAppEventHandler, NULL);
    MCMGR_RegisterEvent(kMCMGR_FreeRtosMessageBuffersEvent, FreeRtosMessageEventHandler, NULL);

    // Start up the remote core.
    // Provide the address of the P->S message queue so that the remote core can
    // receive messages from this core.
    printf("Starting M4\r\n");
    MCMGR_StartCore(kMCMGR_Core1, (void*)CORE1_BOOT_ADDRESS, reinterpret_cast<uint32_t>(p2s_message_buffer), kMCMGR_Start_Asynchronous);

    // Wait for the first event from the remote core.
    while (!remote_alive) { vTaskDelay(pdMS_TO_TICKS(500)); }
    printf("M4 started\r\n");

    // Send the address of the S->P message queue through the P->S queue.
    // That'll let the remote core send messages back to us.
    size_t tx_bytes;
    tx_bytes = xMessageBufferSend(p2s_message_buffer->message_buffer, &s2p_message_buffer, sizeof(s2p_message_buffer), portMAX_DELAY);
    if (tx_bytes == 0) {
        printf("Failed to send s2p buffer address\r\n");
    }

    valiant::StreamBuffer *m4_console_buffer = valiant::ConsoleM7::GetSingleton()->GetM4ConsoleBufferPtr();
    tx_bytes = xMessageBufferSend(p2s_message_buffer->message_buffer, &m4_console_buffer, sizeof(m4_console_buffer), portMAX_DELAY);
    if (tx_bytes == 0) {
        printf("Failed to send console buffer address\r\n");
    }

    while (!xMessageBufferIsEmpty(p2s_message_buffer->message_buffer)) { taskYIELD(); }

    // TODO(atv): We don't do anything else here, at the moment, so let's suspend.
    // In the future, this task should have some sort of queue that it blocks on that accepts
    // messages for the other core.
    vTaskSuspend(NULL);
    while (true) {
        taskYIELD();
    }
}

namespace valiant {

void IPCInit() {
    if (m4_binary_start != 0xdeadbeef) {
        xTaskCreate(ipc_task, "ipc_task", configMINIMAL_STACK_SIZE * 10, NULL, IPC_TASK_PRIORITY, NULL);
    }
}

}  // namespace valiant
