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

#ifndef Arduino_h
#define Arduino_h

// clang-format off
#include <api/ArduinoAPI.h>
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pxp.h"
#include "HardwareSerial.h"
#include "pins_arduino.h"
// clang-format on

#define interrupts() portENABLE_INTERRUPTS()
#define noInterrupts() portDISABLE_INTERRUPTS()

using namespace arduino;
using namespace coralmicro::arduino;

void analogReadResolution(int bits);
void analogWriteResolution(int bits);

extern coralmicro::arduino::HardwareSerial Serial;

#endif  // Arduino_h
