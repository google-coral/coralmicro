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

#ifndef LIBS_BASE_CONSOLE_M7_H_
#define LIBS_BASE_CONSOLE_M7_H_

#include <array>

#include "libs/base/ipc_message_buffer.h"
#include "libs/cdc_acm/cdc_acm.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"

namespace coralmicro {

class ConsoleM7 {
 public:
  static ConsoleM7* GetSingleton() {
    static ConsoleM7 console;
    return &console;
  }
  void Init(bool init_tx, bool init_rx);
  IpcStreamBuffer* GetM4ConsoleBufferPtr();
  void Write(char* buffer, int size);
  // NOTE: This reads from the internal buffer, not directly from a serial
  // device.
  int Read(char* buffer, int size);
  size_t available() { return rx_buffer_available_; }
  uint8_t peek() {
    return (rx_buffer_available_ ? rx_buffer_[rx_buffer_read_] : -1);
  }

  void EmergencyWrite(const char* fmt, ...);

 private:
  struct ConsoleMessage {
    int len;
    uint8_t* str;
#ifdef BLOCKING_PRINTF
    SemaphoreHandle_t semaphore;
    StaticSemaphore_t semaphore_storage;
#endif
  };

  static void StaticM4ConsoleTaskFn(void* param) {
    GetSingleton()->M4ConsoleTaskFn(param);
  }
  void M4ConsoleTaskFn(void* param);

  static void StaticM7ConsoleTaskTxFn(void* param) {
    GetSingleton()->M7ConsoleTaskTxFn(param);
  }
  void M7ConsoleTaskTxFn(void* param);

  static void StaticM7ConsoleTaskRxFn(void* param) {
    GetSingleton()->M7ConsoleTaskRxFn(param);
  }
  void M7ConsoleTaskRxFn(void* param);

  ConsoleM7() = default;
  ConsoleM7(const ConsoleM7&) = delete;
  ConsoleM7& operator=(const ConsoleM7&) = delete;

  QueueHandle_t console_queue_ = nullptr;
  CdcAcm cdc_acm_;

  IpcStreamBuffer* m4_console_buffer_ = nullptr;
  static constexpr size_t kM4ConsoleBufferBytes = 128;
  static constexpr size_t kM4ConsoleBufferSize =
      kM4ConsoleBufferBytes + sizeof(IpcStreamBuffer);
  static uint8_t m4_console_buffer_storage_[kM4ConsoleBufferSize];

  static constexpr size_t kRxBufferSize = 64;
  std::array<uint8_t, kRxBufferSize> rx_buffer_;

  static constexpr size_t kEmergencyBufferSize = 256;
  std::array<char, kEmergencyBufferSize> emergency_buffer_;
  size_t rx_buffer_read_ = 0, rx_buffer_write_ = 0, rx_buffer_available_ = 0;
  SemaphoreHandle_t rx_mutex_;

  TaskHandle_t tx_task_, rx_task_;
};

}  // namespace coralmicro

#endif  // LIBS_BASE_CONSOLE_M7_H_
