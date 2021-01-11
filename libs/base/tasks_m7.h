#ifndef __LIBS_BASE_TASKS_M7_H__
#define __LIBS_BASE_TASKS_M7_H__

#define IPC_TASK_PRIORITY       (configMAX_PRIORITIES - 1)
#define CONSOLE_TASK_PRIORITY   (configMAX_PRIORITIES - 1)
#define APP_TASK_PRIORITY       (configMAX_PRIORITIES - 1)

#if defined(__cplusplus)
static_assert(IPC_TASK_PRIORITY < configMAX_PRIORITIES);
static_assert(IPC_TASK_PRIORITY >= 0);

static_assert(CONSOLE_TASK_PRIORITY < configMAX_PRIORITIES);
static_assert(CONSOLE_TASK_PRIORITY >= 0);

static_assert(APP_TASK_PRIORITY < configMAX_PRIORITIES);
static_assert(APP_TASK_PRIORITY >= 0);
#endif

#endif  // __LIBS_BASE_TASKS_M7_H__
