#ifndef _APPS_RACKTEST_RACK_TEST_IPC_H_
#define _APPS_RACKTEST_RACK_TEST_IPC_H_

#include "libs/base/message_buffer.h"

enum class RackTestAppMessageType : uint8_t {
    XOR = 0,
};

struct RackTestAppMessage {
    RackTestAppMessageType message_type;
    union {
        uint32_t xor_value;
    } message;
};
static_assert(sizeof(RackTestAppMessage) <= valiant::ipc::kMessageBufferDataSize);


#endif  // _APPS_RACKTEST_RACK_TEST_IPC_H_