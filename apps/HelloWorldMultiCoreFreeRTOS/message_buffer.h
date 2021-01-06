#ifndef __APPS_HELLOWORLDMULTICOREFREERTOS_MESSAGE_BUFFER_H_
#define __APPS_HELLOWORLDMULTICOREFREERTOS_MESSAGE_BUFFER_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/message_buffer.h"

namespace valiant {

struct MessageBuffer {
    MessageBufferHandle_t message_buffer;
    StaticMessageBuffer_t static_message_buffer;
    size_t len;
    uint8_t message_buffer_storage[];
};

}  // namespace valiant

#endif  // __APPS_HELLOWORLDMULTICOREFREERTOS_MESSAGE_BUFFER_H_
