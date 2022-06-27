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

#ifndef LIBS_BASE_TASKS_H_
#define LIBS_BASE_TASKS_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"

#if (__CORTEX_M == 7)
#include "libs/base/tasks_m7.h"
#elif (__CORTEX_M == 4)
#include "libs/base/tasks_m4.h"
#else
#error "__CORTEX_M not defined"
#endif

#endif  // LIBS_BASE_TASKS_H_
