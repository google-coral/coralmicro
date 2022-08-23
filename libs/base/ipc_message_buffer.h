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

#ifndef LIBS_BASE_IPC_MESSAGE_BUFFER_H_
#define LIBS_BASE_IPC_MESSAGE_BUFFER_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"

namespace coralmicro {
// @cond Do not generate docs
// The type of message that may be sent in an `IpcSystemMessage`.
enum class IpcSystemMessageType : uint8_t {
  // A message with a pointer to a console buffer.
  kConsoleBufferPtr,
};

// System message to be sent from `IpcM4` or `IpcM7`.
struct IpcSystemMessage {
  // Identifier for the type of message, which will be a `kConsoleBufferPtr`,
  // which is a byte.
  IpcSystemMessageType type;
  // Pointer to console buffer.
  union {
    void* console_buffer_ptr;
  } message;
} __attribute__((packed));
// @endcond

// The types of message that may be sent in an `IpcMessage`.
enum class IpcMessageType : uint8_t {
  // Internal use only.
  kSystem,
  // A custom app message with a byte array of size
  // `kIpcMessageBufferDataSize` (127).
  kApp,
};

// Size of the byte array containing a message.
inline constexpr size_t kIpcMessageBufferDataSize = 127;

// A message to be sent with `Ipc::SendMessage()` (using either `IpcM4` or
// `IpcM7`).
//
// The `message` union is designed to support two types of message, but you
// should always use an "app" message. So you should set `type` to
// `IpcMessageType::kApp` and then populate `data` with a custom data format
// that both processes know how to read/write.
//
// For an example, see `examples/multi_core_ipc/`.
struct IpcMessage {
  // Identifier for the type of message (apps should always use `kApp`).
  IpcMessageType type;
  // The message to be sent (`system` or `data`).
  union {
    // Internal use only.
    IpcSystemMessage system;
    // A byte array, which should be a structured data format that's defined
    // by the app, but limited to size `kIpcMessageBufferDataSize` (127 bytes).
    uint8_t data[kIpcMessageBufferDataSize];
  } message;
} __attribute__((packed));

// @cond Do not generate docs
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
// @endcond

static_assert(sizeof(IpcSystemMessage) <= kIpcMessageBufferDataSize);

}  // namespace coralmicro

#endif  // LIBS_BASE_IPC_MESSAGE_BUFFER_H_
