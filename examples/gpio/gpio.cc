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

#include "libs/base/gpio.h"

#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Toggles each available GPIO on the two 12-pin headers.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e gpio
//
// These GPIOs are not powerful enough to drive an LED,
// but you can observe the changes with a multimeter.

namespace coralmicro {
namespace {

[[noreturn]] void Main() {
  printf("GPIO Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  constexpr Gpio kGpiosToTest[] = {
      kSpiCs, kSpiSck, kSpiSdo,  kSpiSdi,  kSda6, kScl1, kSda1,
      kAA,    kAB,     kUartCts, kUartRts, kPwm1, kPwm0, kScl6,
  };

  for (auto gpio : kGpiosToTest) {
    GpioSetMode(gpio, GpioMode::kOutput);
  }

  printf("Periodically toggling header GPIOs...\r\n");

  bool high = true;
  while (true) {
    for (auto gpio : kGpiosToTest) {
      GpioSet(gpio, high);
    }
    high = !high;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
