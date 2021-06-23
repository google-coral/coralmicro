#include <Arduino.h>
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

extern "C" void app_main(void *param) {
    setup();
    while (true) {
        loop();
        portYIELD();
    }
    vTaskSuspend(NULL);
}