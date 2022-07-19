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
// Singleton object that implements an IPC to read and write from the M4 core
// so that the M4 core can communicate with the M7 core.
// The M4 cannot start processes, including starting the M4 core,
// independently  and needs commands from the M7 core.
// So, you use the IpcM7 to write messages from the M7 core
// and then use the IpcM4 to read and interpret the message.
// You use the method `RegisterAppMessageHandler()` to define
// how to interpret the message and run a process.
// Then you use the method `SendMessage()` to respond to the
// M7 core with the result of the process run by the M4 core.
class IpcM4 : public Ipc {
 public:
  // Creates the static IpcM4 object the first time `GetSingleton()`
  // is called and returns the reference to the IpcM7 singleton.
  //
  // @return A reference to the singleton IpcM4 object.
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
