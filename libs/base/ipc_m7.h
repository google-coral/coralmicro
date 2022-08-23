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

#ifndef LIBS_BASE_IPC_M7_H_
#define LIBS_BASE_IPC_M7_H_

#include <cstdint>

#include "libs/base/ipc.h"

namespace coralmicro {
// Singleton object that provides IPC on the M7 core so it can start the M4 core
// and relay messages with the M4.
//
// The M4 can not operate independent from the M7: the M7 must
// start the M4 with `StartM4()`, and then the M7 task may suspend itself.
//
// The M7 can send messages to the M4 with `SendMessage()` and register
// a callback to receive messages from the M4 with
// `RegisterAppMessageHandler()`.
class IpcM7 : public Ipc {
 public:
  // Gets the `IpcM7` singleton that can start the M4 and perform IPC with the
  // M4.
  //
  // @return A reference to the singleton `IpcM7` object.
  static IpcM7* GetSingleton() {
    static IpcM7 ipc;
    return &ipc;
  }

  // @cond Do not generate docs.
  void Init() override;
  // @endcond

  // Starts the M4 core, invoking the `app_main()` function in the executable
  // declared with the CMake `add_executable_m4()` command.
  void StartM4();

  // Checks if the M4 core is alive.
  //
  // @param millis The amount of time (in milliseconds) to wait for a response
  // from the M4 core.
  // @return True if the M4 core signals that it is ready to preform a
  // task or preforming a task within the millis time limit, false otherwise.
  bool M4IsAlive(uint32_t millis);

  // Checks if the M4 core is running a process.
  //
  // @return True if the M4 is running a process, false otherwise.
  static bool HasM4Application();

 protected:
  void TxTaskFn() override;

 private:
  static void StaticRemoteAppEventHandler(uint16_t eventData, void* context) {
    GetSingleton()->RemoteAppEventHandler(eventData, context);
  }
  void RemoteAppEventHandler(uint16_t eventData, void* context);
  void HandleSystemMessage(const IpcSystemMessage& message) override;

  static constexpr size_t kMessageBufferSize = 8 * sizeof(IpcMessage);
  static uint8_t
      tx_queue_storage_[kMessageBufferSize + sizeof(IpcMessageBuffer)]
      __attribute__((section(".noinit.$rpmsg_sh_mem")));
  static uint8_t
      rx_queue_storage_[kMessageBufferSize + sizeof(IpcMessageBuffer)]
      __attribute__((section(".noinit.$rpmsg_sh_mem")));
};
}  // namespace coralmicro

#endif  // LIBS_BASE_IPC_M7_H_
