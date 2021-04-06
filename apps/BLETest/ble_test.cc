#include "apps/BLETest/ble_hello_sensor.h"
#include "libs/base/filesystem.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"

extern "C" wiced_result_t wiced_wlan_connectivity_init(void);
extern unsigned char brcm_patchram_buf[];
extern unsigned int brcm_patch_ram_length;

extern "C" void app_main(void *param) {
    gpio_pin_config_t bt_reg_on_config;
    bt_reg_on_config.direction = kGPIO_DigitalOutput;
    bt_reg_on_config.outputLogic = 0;
    bt_reg_on_config.interruptMode = kGPIO_NoIntmode;
    GPIO_PinInit(GPIO10, 2, &bt_reg_on_config);
    printf("Read patchram to sdram\r\n");
    size_t brcm_patchram_size = brcm_patch_ram_length;
    if (!valiant::filesystem::ReadToMemory("/firmware/BCM4345C0_003.001.025.0144.0266.1MW.hcd", brcm_patchram_buf, &brcm_patchram_size)) {
        printf("Reading patchram failed\r\n");
        vTaskSuspend(NULL);
    }
    printf("Done reading patchram\r\n");
    wiced_wlan_connectivity_init();
    hello_sensor_start();
    vTaskSuspend(NULL);
}
