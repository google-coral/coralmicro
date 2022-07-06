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
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <cstdio>

// Reads analog input from ADC1 channel B (pin 4 on the left-side header)
// and writes it to the DAC (pin 9 on the right-side header)
// Note: The DAC outputs a max of 1.8V

// [start-sphinx-snippet:dac-adc]
extern "C" void app_main(void *param) {
    coralmicro::analog::Init(coralmicro::analog::Device::kAdc1);
    coralmicro::analog::Init(coralmicro::analog::Device::kDac1);
    coralmicro::analog::ADCConfig config;
    coralmicro::analog::CreateConfig(
        config,
        coralmicro::analog::Device::kAdc1, 0,
        coralmicro::analog::Side::kB,
        false
    );
    coralmicro::analog::EnableDAC(true);
    while (true) {
        uint16_t val = coralmicro::analog::ReadADC(config);
        coralmicro::analog::WriteDAC(val);
        printf("ADC val: %u\r\n", val);
    }
}
// [end-sphinx-snippet:dac-adc]
