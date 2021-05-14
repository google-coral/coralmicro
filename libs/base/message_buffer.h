#ifndef __APPS_HELLOWORLDMULTICOREFREERTOS_MESSAGE_BUFFER_H_
#define __APPS_HELLOWORLDMULTICOREFREERTOS_MESSAGE_BUFFER_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"

namespace valiant {

namespace ipc {

enum class SystemMessageType : uint8_t {
    CONSOLE_BUFFER_PTR,
};

struct SystemMessage {
    SystemMessageType type;
    union {
        void *console_buffer_ptr;
    } message;
} __attribute__((packed));

enum class MessageType : uint8_t {
    SYSTEM,
    APP,
};

constexpr size_t kMessageBufferDataSize = 127;
struct Message {
    MessageType type;
    union {
        SystemMessage system;
        uint8_t data[kMessageBufferDataSize];
    } message;
} __attribute__((packed));

struct MessageBuffer {
    MessageBufferHandle_t message_buffer;
    StaticMessageBuffer_t static_message_buffer;
    uint8_t message_buffer_storage[];
};

struct StreamBuffer {
    StreamBufferHandle_t stream_buffer;
    StaticStreamBuffer_t static_stream_buffer;
    uint8_t stream_buffer_storage[];
};

#if defined(__cplusplus)
static_assert(sizeof(SystemMessage) <= kMessageBufferDataSize);
#endif

}  // namespace ipc
}  // namespace valiant

#endif  // __APPS_HELLOWORLDMULTICOREFREERTOS_MESSAGE_BUFFER_H_
