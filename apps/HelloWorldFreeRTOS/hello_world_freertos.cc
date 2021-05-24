#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

extern "C" void app_main(void *param) {
    printf("Hello world FreeRTOS.\r\n");

    bool on = true;
    while (true) {
        on = !on;
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kPowerLED, on);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
