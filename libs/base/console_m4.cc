#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "libs/base/console_m4.h"
#include <unistd.h>

static valiant::StreamBuffer *console_buffer = nullptr;

extern "C" int _write(int handle, char *buffer, int size) {
    if ((handle != STDOUT_FILENO) && (handle != STDERR_FILENO)) {
        return -1;
    }

    if (console_buffer) {
        xStreamBufferSend(console_buffer->stream_buffer, buffer, size, portMAX_DELAY);
    }

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

void SetM4ConsoleBufferPtr(StreamBuffer* buffer) {
    console_buffer = buffer;
}

}  // namespace valiant
