#include "libs/base/tasks_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <cstdio>

extern "C" void app_main(void *param) {
    printf("Hello world FreeRTOS.\r\n");
    while (true) {
        taskYIELD();
    }
}
