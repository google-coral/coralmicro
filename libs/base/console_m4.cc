#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "libs/base/console_m4.h"
#include <unistd.h>

static valiant::ipc::StreamBuffer *console_buffer = nullptr;

extern "C" int DbgConsole_SendDataReliable(uint8_t*, size_t);
extern "C" int _write(int handle, char *buffer, int size) {
    if ((handle != STDOUT_FILENO) && (handle != STDERR_FILENO)) {
        return -1;
    }

    if (console_buffer) {
        xStreamBufferSend(console_buffer->stream_buffer, buffer, size, portMAX_DELAY);
    }
    DbgConsole_SendDataReliable(reinterpret_cast<uint8_t*>(buffer), size);

    return size;
}

extern "C" int _read(int handle, char *buffer, int size) {
    if (handle != STDIN_FILENO) {
        return -1;
    }

    return -1;
}

namespace valiant {

void ConsoleInit() {
}

void SetM4ConsoleBufferPtr(ipc::StreamBuffer* buffer) {
    console_buffer = buffer;
}

}  // namespace valiant
