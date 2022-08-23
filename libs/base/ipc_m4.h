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

#ifndef LIBS_BASE_IPC_M4_H_
#define LIBS_BASE_IPC_M4_H_

#include "libs/base/ipc.h"

namespace coralmicro {
// Singleton object that provides IPC on the M4 core to relay messages
// with the M7 core.
//
// The M4 can not operate independent from the M7: the M7 must start the
// M4 with `IpcM7::StartM4()`, and then the M7 task may suspend itself.
//
// The M4 can send messages to the M7 with `SendMessage()` and register
// a callback to receive messages from the M7 with
// `RegisterAppMessageHandler()`.
class IpcM4 : public Ipc {
 public:
  // Gets the `IpcM4` singleton to use for IPC with the M7.
  //
  // @return A reference to the singleton `IpcM4` object.
  static IpcM4* GetSingleton() {
    static IpcM4 ipc;
    return &ipc;
  }
  // @cond Do not generate docs.
  void Init() override;
  // @endcond

 protected:
  void RxTaskFn() override;

 private:
  void HandleSystemMessage(const IpcSystemMessage& message) override;
};
}  // namespace coralmicro

#endif  // LIBS_BASE_IPC_M4_H_
