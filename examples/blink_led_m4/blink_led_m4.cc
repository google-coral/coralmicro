#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M4.\r\n");
    bool on = true;
    while (true) {
        on = !on;
        coral::micro::led::Set(coral::micro::led::LED::kPower, on);
        coral::micro::led::Set(coral::micro::led::LED::kUser, on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
