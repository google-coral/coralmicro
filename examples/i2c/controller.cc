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

#include <memory>
#include <vector>

#include "libs/base/check.h"
#include "libs/base/i2c.h"
#include "libs/base/led.h"
#include "libs/base/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Runs the board as an I2C controller device.
// See the local README.

namespace coralmicro {
namespace {
// [start-sphinx-snippet:i2c-controller]
void Main() {
  printf("i2c Controller Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  auto config = I2cGetDefaultConfig(coralmicro::I2c::kI2c1);
  I2cInitController(config);

  std::string serial = GetSerialNumber();
  constexpr int kTargetAddress = 0x42;
  int kTransferSize = serial.length();

  printf("Writing our serial number to the remote device...\r\n");
  CHECK(I2cControllerWrite(config, kTargetAddress,
                           reinterpret_cast<uint8_t*>(serial.data()),
                           kTransferSize));
  auto buffer = std::vector<uint8_t>(kTransferSize, 0);

  printf("Reading back our serial number from the remote device...\r\n");
  CHECK(
      I2cControllerRead(config, kTargetAddress, buffer.data(), kTransferSize));
  CHECK(memcmp(buffer.data(), serial.data(), kTransferSize) == 0);
  printf("Readback of data from target device matches written data!\r\n");
}
// [end-sphinx-snippet:i2c-controller]
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
