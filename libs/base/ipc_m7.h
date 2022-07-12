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
class IpcM7 : public Ipc {
   public:
    static IpcM7* GetSingleton() {
        static IpcM7 ipc;
        return &ipc;
    }

    void Init() override;
    void StartM4();
    bool M4IsAlive(uint32_t millis);
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
