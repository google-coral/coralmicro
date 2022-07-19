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

#include "libs/base/analog.h"

#include <cstdio>

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Reads analog input from ADC1 channel B (pin 4 on the left-side header)
// and writes it to the DAC (pin 9 on the right-side header)
// Note: The DAC outputs a max of 1.8V

// [start-sphinx-snippet:dac-adc]
extern "C" void app_main(void *param) {
  coralmicro::AdcInit(coralmicro::AdcDevice::kAdc1);
  coralmicro::DacInit();
  coralmicro::AdcConfig config;
  coralmicro::AdcCreateConfig(config, coralmicro::AdcDevice::kAdc1, 0,
                              coralmicro::AdcSide::kB,
                              /*differential=*/false);
  coralmicro::DacEnable(true);
  while (true) {
    uint16_t val = coralmicro::AdcRead(config);
    coralmicro::DacWrite(val);
    printf("ADC val: %u\r\n", val);
  }
}
// [end-sphinx-snippet:dac-adc]
