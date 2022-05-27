#include "libs/base/mutex.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

extern "C" void app_main(void *param) {
    while(true) {
        coral::micro::MulticoreMutexLock lock(0);
        printf("[M4] Hello.\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
