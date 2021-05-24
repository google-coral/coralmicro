#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"

namespace valiant {
namespace gpio {

static SemaphoreHandle_t gpio_semaphore;
static StaticSemaphore_t gpio_semaphore_static;

static GPIO_Type* PinNameToModule[Gpio::kCount] = {
    [Gpio::kPowerLED] = GPIO13,
    [Gpio::kUserLED] = GPIO13,
    [Gpio::kEdgeTpuPgood] = GPIO8,
    [Gpio::kEdgeTpuReset] = GPIO8,
    [Gpio::kEdgeTpuPmic] = GPIO8,
    [Gpio::kBtRegOn] = GPIO10,
};

static uint32_t PinNameToPin[Gpio::kCount] = {
    [Gpio::kPowerLED] = 5,
    [Gpio::kUserLED] = 6,
    [Gpio::kEdgeTpuPgood] = 26,
    [Gpio::kEdgeTpuReset] = 24,
    [Gpio::kEdgeTpuPmic] = 25,
    [Gpio::kBtRegOn] = 2,
};

static gpio_pin_config_t PinNameToConfig[Gpio::kCount] = {
    [Gpio::kPowerLED] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kUserLED] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kEdgeTpuPgood] = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kEdgeTpuReset] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kEdgeTpuPmic] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kBtRegOn] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
};

void Init() {
    gpio_semaphore = xSemaphoreCreateMutexStatic(&gpio_semaphore_static);

    for (int i = 0; i < Gpio::kCount; ++i) {
        GPIO_PinInit(
            PinNameToModule[i],
            PinNameToPin[i],
            &PinNameToConfig[i]
        );
    }
}

void SetGpio(Gpio gpio, bool enable) {
    xSemaphoreTake(gpio_semaphore, portMAX_DELAY);
    GPIO_PinWrite(
        PinNameToModule[gpio],
        PinNameToPin[gpio],
        enable
    );
    xSemaphoreGive(gpio_semaphore);
}

bool GetGpio(Gpio gpio) {
    bool ret;
    xSemaphoreTake(gpio_semaphore, portMAX_DELAY);
    ret = !!GPIO_PinRead(PinNameToModule[gpio], PinNameToPin[gpio]);
    xSemaphoreGive(gpio_semaphore);
    return ret;
}

}  // namespace gpio
}  // namespace valiant