// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _EXAMPLE_MESSAGE_PASSING_EXAMPlE_MESSAGE_H_
#define _EXAMPLE_MESSAGE_PASSING_EXAMPlE_MESSAGE_H_

#include "libs/base/ipc_message_buffer.h"

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

    static_assert(sizeof(ExampleAppMessage) <= coralmicro::kIpcMessageBufferDataSize);

} // namespace mp_example

#endif  // _EXAMPLE_MESSAGE_PASSING_EXAMPlE_MESSAGE_H_
