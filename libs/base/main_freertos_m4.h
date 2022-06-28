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

#ifndef LIBS_BASE_MAIN_FREERTOS_M4_H_
#define LIBS_BASE_MAIN_FREERTOS_M4_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#if defined(__cplusplus)
extern "C" {
#endif

lpi2c_rtos_handle_t* I2C5Handle();

#if defined(__cplusplus)
}
#endif

#endif  // LIBS_BASE_MAIN_FREERTOS_M4_H_
