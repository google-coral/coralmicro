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
