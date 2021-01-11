#include "libs/base/message_buffer.h"
#include "libs/base/tasks_m7.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include <cstdio>
#include <unistd.h>

SemaphoreHandle_t console_mtx;
extern "C" int DbgConsole_SendDataReliable(uint8_t*, size_t);
extern "C" int _write(int handle, char *buffer, int size) {
    if ((handle != STDOUT_FILENO) && (handle != STDERR_FILENO)) {
        return -1;
    }

    if (xSemaphoreTake(console_mtx, portMAX_DELAY) == pdTRUE) {
        DbgConsole_SendDataReliable((uint8_t*)buffer, size);
        xSemaphoreGive(console_mtx);
        return size;
    }

    return -1;
}

extern "C" int _read(int handle, char *buffer, int size) {
    if (handle != STDIN_FILENO) {
        return -1;
    }

    return -1;
}

namespace valiant {

static constexpr size_t kM4ConsoleBufferBytes = 128;
static StreamBuffer *m4_console_buffer = nullptr;
static uint8_t m4_console_buffer_storage[sizeof(StreamBuffer) + kM4ConsoleBufferBytes] __attribute__((section(".noinit.$rpmsg_sh_mem")));

void console_task(void *param) {
    size_t rx_bytes;
    char buf[16];
    while (true) {
        rx_bytes = xStreamBufferReceive(m4_console_buffer->stream_buffer, buf, sizeof(buf), pdMS_TO_TICKS(10));
        if (rx_bytes > 0) {
            for (size_t i = 0; i < rx_bytes; ++i) {
                putchar(buf[i]);
            }
        }
    }
}

void ConsoleInit() {
    m4_console_buffer = reinterpret_cast<StreamBuffer*>(m4_console_buffer_storage);
    m4_console_buffer->stream_buffer =
        xStreamBufferCreateStatic(kM4ConsoleBufferBytes, 1, m4_console_buffer->stream_buffer_storage, &m4_console_buffer->static_stream_buffer);
    console_mtx = xSemaphoreCreateMutex();
    xTaskCreate(console_task, "console_task", configMINIMAL_STACK_SIZE * 10, NULL, CONSOLE_TASK_PRIORITY, NULL);
}

StreamBuffer* GetM4ConsoleBufferPtr() {
    return m4_console_buffer;
}

}  // namespace valiant
