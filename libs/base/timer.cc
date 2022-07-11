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

#include "libs/base/timer.h"

#include <climits>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpt.h"

namespace coralmicro {
namespace {
// Number of times the microseconds counter has rolled over.
uint32_t micros_rollover = 0;
} // namespace

void TimerInit() {
    gpt_config_t gpt_config;
    GPT_GetDefaultConfig(&gpt_config);
    gpt_config.clockSource = kGPT_ClockSource_Periph;

    uint32_t root_freq = CLOCK_GetRootClockFreq(kCLOCK_Root_Gpt1);

    GPT_Init(GPT1, &gpt_config);
    GPT_EnableInterrupts(GPT1, kGPT_RollOverFlagInterruptEnable);
    GPT_ClearStatusFlags(GPT1, kGPT_RollOverFlag);
    // Set the divider to get us close to 1MHz.
    GPT_SetClockDivider(GPT1, root_freq / 1000000);
    EnableIRQ(GPT1_IRQn);

    GPT_StartTimer(GPT1);
}

uint32_t TimerMillis() {
    return (micros_rollover * (UINT_MAX / 1000)) +
           (GPT_GetCurrentTimerCount(GPT1) / 1000);
}

uint32_t TimerMicros() { return GPT_GetCurrentTimerCount(GPT1); }

}  // namespace coralmicro

extern "C" void GPT1_IRQHandler() {
    if (GPT_GetStatusFlags(GPT1, kGPT_RollOverFlag)) {
        GPT_ClearStatusFlags(GPT1, kGPT_RollOverFlag);
        coralmicro::micros_rollover++;
    }
}

extern "C" uint32_t vPortGetRunTimeCounterValue() {
    return coralmicro::TimerMicros();
}
