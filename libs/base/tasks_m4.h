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

#ifndef LIBS_BASE_TASKS_M4_H_
#define LIBS_BASE_TASKS_M4_H_

#include "FreeRTOSConfig.h"

#define IPC_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define CONSOLE_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define APP_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define CAMERA_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define PMIC_TASK_PRIORITY (configMAX_PRIORITIES - 1)

#if defined(__cplusplus)
#define VALIDATE_TASK_PRIORITY(prio)                            \
    static_assert(0 <= (prio) && (prio) < configMAX_PRIORITIES, \
                  "Invalid value for task priority")

VALIDATE_TASK_PRIORITY(IPC_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(CONSOLE_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(CONSOLE_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(CAMERA_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(PMIC_TASK_PRIORITY);
#endif  // __cplusplus

#endif  // LIBS_BASE_TASKS_M4_H_
