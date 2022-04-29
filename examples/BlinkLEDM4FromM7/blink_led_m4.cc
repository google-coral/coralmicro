#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M4.\r\n");
    bool on = true;
    while (true) {
        on = !on;
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kPowerLED, on);
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kUserLED, on);
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kTpuLED, on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
