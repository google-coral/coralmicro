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

#include <vector>

#include "libs/base/check.h"
#include "libs/base/i2c.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Runs the board as an I2C target device.
// See the local README.

namespace coralmicro {
namespace {
void Main() {
  printf("i2c Target Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  constexpr int kTargetAddress = 0x42;
  constexpr int kTransferSize = 16;
  auto config = I2cGetDefaultConfig(coralmicro::I2c::kI2c6);

  auto buffer = std::vector<uint8_t>(kTransferSize, 0);

  CHECK(I2cInitTarget(
      config, kTargetAddress,
      [buffer](I2cConfig* config, lpi2c_target_transfer_t* transfer) mutable {
        switch (transfer->event) {
          case kLPI2C_SlaveTransmitEvent:
          case kLPI2C_SlaveReceiveEvent:
            transfer->data = buffer.data();
            transfer->dataSize = kTransferSize;
            break;
          default:
            break;
        }
      }));

  // The `buffer` variable we capture into the callback is on the stack -- so
  // just suspend here instead of returning.
  vTaskSuspend(nullptr);
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
