#include <Arduino.h>
#include "wiring_private.h"

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

extern "C" void app_main(void *param) {
    wiringInit();
    setup();
    while (true) {
        loop();
        portYIELD();
    }
    vTaskSuspend(NULL);
}