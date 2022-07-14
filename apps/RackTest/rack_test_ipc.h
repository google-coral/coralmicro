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

#ifndef APPS_RACKTEST_RACK_TEST_IPC_H_
#define APPS_RACKTEST_RACK_TEST_IPC_H_

#include "libs/base/ipc_message_buffer.h"

enum class RackTestAppMessageType : uint8_t {
  kXor = 0,
  kCoreMark,
};

struct RackTestAppMessage {
  RackTestAppMessageType message_type;
  union {
    uint32_t xor_value;
    char* buffer_ptr;
  } message;
};
static_assert(sizeof(RackTestAppMessage) <=
              coralmicro::kIpcMessageBufferDataSize);

#endif  // APPS_RACKTEST_RACK_TEST_IPC_H_
