#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M4.\r\n");
    bool on = true;
    while (true) {
        on = !on;
        // TODO(vunam): switch over to using the valiant::gpio::SetGpio interface for the m4.
        GPIO_PinWrite(GPIO9, 1, on);
        GPIO_PinWrite(GPIO13, 5, on);
        GPIO_PinWrite(GPIO13, 6, on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
