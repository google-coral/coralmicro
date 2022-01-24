#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M4.\r\n");
    bool on = true;
    while (true) {
        on = !on;
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kPowerLED, on);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, on);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kTpuLED, on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
