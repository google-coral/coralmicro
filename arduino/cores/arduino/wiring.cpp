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

#include "Arduino.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.h"
#include "wiring_private.h"

void wiringInit() { wiringAnalogInit(); }

void delay(unsigned long ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

void delayMicroseconds(unsigned int us) {
  SDK_DelayAtLeastUs(us, CLOCK_GetFreq(kCLOCK_CpuClk));
}

unsigned long millis() {
  return static_cast<unsigned long>(coralmicro::TimerMillis());
}

unsigned long micros() {
  return static_cast<unsigned long>(coralmicro::TimerMicros());
}
