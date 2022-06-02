#ifndef _EXAMPLE_MESSAGE_PASSING_EXAMPlE_MESSAGE_H_
#define _EXAMPLE_MESSAGE_PASSING_EXAMPlE_MESSAGE_H_

#include "libs/base/message_buffer.h"

namespace mp_example {

    enum class ExampleMessageType : uint8_t {
        LED_STATUS,
        ACKNOWLEDGED,
    };

    enum class LEDStatus : uint8_t {
        ON,
        OFF,
    };

    struct ExampleAppMessage {
        ExampleMessageType type;
        LEDStatus led_status;
    } __attribute__((packed));

    static_assert(sizeof(ExampleAppMessage) <= coral::micro::ipc::kMessageBufferDataSize);

} // namespace mp_example

#endif  // _EXAMPLE_MESSAGE_PASSING_EXAMPlE_MESSAGE_H_