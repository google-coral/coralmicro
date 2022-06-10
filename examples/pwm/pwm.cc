#include "libs/base/pwm.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"

#include <cstdio>

// Runs simple PWM cycles with pins PWM_A (pin 10 on the left-side header)
// and PWM_B (pin 9 on the left-side header).
// Note: These pins output a max of 1.8V

extern "C" void app_main(void *param) {
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_00_FLEXPWM1_PWM0_A, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_01_FLEXPWM1_PWM0_B, 0U);
    printf("i'm so pwm\r\n");
    coral::micro::pwm::PwmModuleConfig config;
    memset(&config, 0, sizeof(config));
    config.base = PWM1;
    config.module = kPWM_Module_0;
    config.A.enabled = true;
    config.A.duty_cycle = 20;
    config.B.enabled = true;
    config.B.duty_cycle = 80;
    coral::micro::pwm::Init(config);
    while (true) {
        coral::micro::pwm::Enable(config, true);
        vTaskDelay(pdMS_TO_TICKS(1000));
        coral::micro::pwm::Enable(config, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
