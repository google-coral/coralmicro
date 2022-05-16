#ifndef LIBS_BASE_CHECK_H_
#define LIBS_BASE_CHECK_H_

#include <cstdio>

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#define CHECK(a)                                                          \
    do {                                                                  \
        if (!(a)) {                                                       \
            printf("%s:%d %s was not true.\r\n", __FILE__, __LINE__, #a); \
            vTaskSuspend(nullptr);                                        \
        }                                                                 \
    } while (0)

#endif  // LIBS_BASE_CHECK_H_
