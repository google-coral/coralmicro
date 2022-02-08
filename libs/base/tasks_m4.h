#ifndef __LIBS_BASE_TASKS_M4_H__
#define __LIBS_BASE_TASKS_M4_H__

#include "libs/FreeRTOS/FreeRTOSConfig.h"
#define IPC_TASK_PRIORITY       (configMAX_PRIORITIES - 1)
#define CONSOLE_TASK_PRIORITY   (configMAX_PRIORITIES - 1)
#define APP_TASK_PRIORITY       (configMAX_PRIORITIES - 1)
#define CAMERA_TASK_PRIORITY        (configMAX_PRIORITIES - 1)
#define PMIC_TASK_PRIORITY          (configMAX_PRIORITIES - 1)

#if defined(__cplusplus)
#define VALIDATE_TASK_PRIORITY(prio) \
    static_assert(prio < configMAX_PRIORITIES, "Invalid value for task priority"); \
    static_assert(prio >= 0, "Invalid value for task priority");

VALIDATE_TASK_PRIORITY(IPC_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(CONSOLE_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(CONSOLE_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(CAMERA_TASK_PRIORITY);
VALIDATE_TASK_PRIORITY(PMIC_TASK_PRIORITY);
#endif

#endif  // __LIBS_BASE_TASKS_M4_H__
