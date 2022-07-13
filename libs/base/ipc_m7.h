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
// Singleton object that implements an IPC to read and write from the M7 core
// so that the M7 core can communicate with the M4 core.
// The IpcM7 is used to send instructions to the M4 core
// because the M4 core can not run processes independently.
// The `StartM4()` method is the only way to start the M4 core.
// The read, defined by `RegisterAppMessageHandler()`, on the M7 IPC object is
// used to confirm success or failure of the result of an M4 process.
class IpcM7 : public Ipc {
   public:
    // Creates the static IpcM7 object the first time `GetSingleton()`
    // is called and returns the reference to the IpcM7 singleton.
    //
    // @return A reference to the singleton IpcM4 object.
    static IpcM7* GetSingleton() {
        static IpcM7 ipc;
        return &ipc;
    }
    // @cond Internal only, do not generate docs.
    void Init() override;
    // @endcond
    void StartM4();
    // Checks if the M4 core is alive.
    //
    // @param millis The amount of time to wait for a response from the M4 core.
    // @return True if the M4 core signals that it is ready to preform a task or preforming a task within the millis time limit,
    // false otherwise.
    bool M4IsAlive(uint32_t millis);
    // Checks if the M4 core is running a process.
    //
    // @return True if the M4 is running a process, false otherwise
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
