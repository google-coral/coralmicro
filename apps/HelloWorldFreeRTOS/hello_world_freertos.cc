#include "libs/base/tasks_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include <cstdio>

extern "C" void app_main(void *param) {
    printf("Hello world FreeRTOS.\r\n");

    gpio_pin_config_t user_led = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO13, 6, &user_led);

    gpio_pin_config_t pwr_led = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO13, 5, &pwr_led);

    bool on = true;
    while (true) {
        on = !on;
        GPIO_PinWrite(GPIO13, 5, on);
        GPIO_PinWrite(GPIO13, 6, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
