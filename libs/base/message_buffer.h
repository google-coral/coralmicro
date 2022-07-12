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

#ifndef LIBS_BASE_MESSAGE_BUFFER_H_
#define LIBS_BASE_MESSAGE_BUFFER_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"

namespace coralmicro {

enum class IpcSystemMessageType : uint8_t {
    kConsoleBufferPtr,
};

struct IpcSystemMessage {
    IpcSystemMessageType type;
    union {
        void* console_buffer_ptr;
    } message;
} __attribute__((packed));

enum class IpcMessageType : uint8_t {
    kSystem,
    kApp,
};

inline constexpr size_t kIpcMessageBufferDataSize = 127;
struct IpcMessage {
    IpcMessageType type;
    union {
        IpcSystemMessage system;
        uint8_t data[kIpcMessageBufferDataSize];
    } message;
} __attribute__((packed));

struct IpcMessageBuffer {
    MessageBufferHandle_t message_buffer;
    StaticMessageBuffer_t static_message_buffer;
    uint8_t message_buffer_storage[];
};

struct IpcStreamBuffer {
    StreamBufferHandle_t stream_buffer;
    StaticStreamBuffer_t static_stream_buffer;
    uint8_t stream_buffer_storage[];
};

static_assert(sizeof(IpcSystemMessage) <= kIpcMessageBufferDataSize);

}  // namespace coralmicro

#endif  // LIBS_BASE_MESSAGE_BUFFER_H_
