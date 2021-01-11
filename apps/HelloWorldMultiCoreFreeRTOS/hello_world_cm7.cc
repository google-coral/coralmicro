#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>
#include <cstring>

extern "C" void app_main(void *param) {
    while (true) {
        printf("[M7] Hello.\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
