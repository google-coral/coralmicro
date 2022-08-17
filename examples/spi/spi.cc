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

#include "libs/base/spi.h"

#include "libs/base/check.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Executes an SPI transaction on LPSPI6 (header pins).
// Connect the SDI and SDO pins to each other to execute this successfully.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e spi

namespace coralmicro {
namespace {
// [start-sphinx-snippet:spi]
void Main() {
  printf("SPI Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  constexpr int kTransferBytes = 256;
  std::array<uint8_t, kTransferBytes> tx_data{};
  std::array<uint8_t, kTransferBytes> rx_data{};
  for (int i = 0; i < kTransferBytes; ++i) {
    tx_data[i] = i;
    rx_data[i] = 0;
  }

  SpiConfig config{};
  SpiGetDefaultConfig(&config);
  CHECK(SpiInit(config));
  CHECK(SpiTransfer(config, tx_data.data(), rx_data.data(), kTransferBytes));
  printf("Executing a SPI transaction.\r\n");
  for (int i = 0; i < kTransferBytes; ++i) {
    CHECK(tx_data[i] == rx_data[i]);
  }
  printf("Transaction success!\r\n");
}
// [end-sphinx-snippet:spi]
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
