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

#include "libs/base/console_m4.h"

#include <unistd.h>

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

namespace {
coralmicro::IpcStreamBuffer* console_buffer = nullptr;
}  // namespace

extern "C" int DbgConsole_SendDataReliable(uint8_t*, size_t);
extern "C" int _write(int handle, char* buffer, int size) {
    if ((handle != STDOUT_FILENO) && (handle != STDERR_FILENO)) {
        return -1;
    }

    if (console_buffer) {
        xStreamBufferSend(console_buffer->stream_buffer, buffer, size,
                          portMAX_DELAY);
    }
    DbgConsole_SendDataReliable(reinterpret_cast<uint8_t*>(buffer), size);
#ifdef BLOCKING_PRINTF
    DbgConsole_Flush();
#endif

    return size;
}

extern "C" int _read(int handle, char* buffer, int size) {
    if (handle != STDIN_FILENO) {
        return -1;
    }

    return -1;
}

namespace coralmicro {

void ConsoleM4Init() {}

void ConsoleM4SetBuffer(IpcStreamBuffer* buffer) {
    console_buffer = buffer;
}

}  // namespace coralmicro
