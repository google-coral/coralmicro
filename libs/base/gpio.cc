#include "libs/base/mutex.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"

namespace valiant {
namespace gpio {
static void IRQHandler(GPIO_Type* gpio, uint32_t pin);
}
}

extern "C" void GPIO13_Combined_0_31_IRQHandler(void) {
    uint32_t pins = GPIO_PortGetInterruptFlags(GPIO13);
    GPIO_PortClearInterruptFlags(GPIO13, pins);
    int i = 0;
    while (pins) {
        if (pins & 1) {
            valiant::gpio::IRQHandler(GPIO13, i);
        }
        ++i;
        pins = pins >> 1;
    }
    SDK_ISR_EXIT_BARRIER;
}

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
    [Gpio::kUserButton] = GPIO13,
    [Gpio::kCameraTrigger] = GPIO8,
    [Gpio::kAntennaSelect] = GPIO11,
};

static uint32_t PinNameToPin[Gpio::kCount] = {
    [Gpio::kPowerLED] = 5,
    [Gpio::kUserLED] = 6,
    [Gpio::kEdgeTpuPgood] = 26,
    [Gpio::kEdgeTpuReset] = 24,
    [Gpio::kEdgeTpuPmic] = 25,
    [Gpio::kBtRegOn] = 2,
    [Gpio::kUserButton] = 3,
    [Gpio::kCameraTrigger] = 27,
    [Gpio::kAntennaSelect] = 7,
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
    [Gpio::kUserButton] = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_IntFallingEdge,
    },
    [Gpio::kCameraTrigger] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kAntennaSelect] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
};

static IRQn_Type PinNameToIRQ[Gpio::kCount] = {
    [Gpio::kPowerLED] = HardFault_IRQn,
    [Gpio::kUserLED] = HardFault_IRQn,
    [Gpio::kEdgeTpuPgood] = HardFault_IRQn,
    [Gpio::kEdgeTpuReset] = HardFault_IRQn,
    [Gpio::kEdgeTpuPmic] = HardFault_IRQn,
    [Gpio::kBtRegOn] = HardFault_IRQn,
    [Gpio::kUserButton] = GPIO13_Combined_0_31_IRQn,
    [Gpio::kCameraTrigger] = HardFault_IRQn,
    [Gpio::kAntennaSelect] = HardFault_IRQn,
};

static GpioCallback IRQHandlers[Gpio::kCount];

void Init() {
    gpio_semaphore = xSemaphoreCreateMutexStatic(&gpio_semaphore_static);
    for (int i = 0; i < Gpio::kCount; ++i) {
        IRQHandlers[i] = nullptr;
        GPIO_PinInit(
            PinNameToModule[i],
            PinNameToPin[i],
            &PinNameToConfig[i]
        );
        if (PinNameToConfig[i].interruptMode != kGPIO_NoIntmode) {
            GPIO_PinSetInterruptConfig(PinNameToModule[i], PinNameToPin[i], PinNameToConfig[i].interruptMode);
            GPIO_PortClearInterruptFlags(PinNameToModule[i], GPIO_PortGetInterruptFlags(PinNameToModule[i]));
            GPIO_PortEnableInterrupts(PinNameToModule[i], 1 << PinNameToPin[i]);
            EnableIRQ(PinNameToIRQ[i]);
            NVIC_SetPriority(PinNameToIRQ[i], 5);
        }
    }
}

void SetGpio(Gpio gpio, bool enable) {
    MutexLock lock(gpio_semaphore);
    GPIO_PinWrite(
        PinNameToModule[gpio],
        PinNameToPin[gpio],
        enable
    );
}

bool GetGpio(Gpio gpio) {
    MutexLock lock(gpio_semaphore);
    return !!GPIO_PinRead(PinNameToModule[gpio], PinNameToPin[gpio]);
}

void RegisterIRQHandler(Gpio gpio, GpioCallback cb) {
    IRQHandlers[gpio] = cb;
}

static void IRQHandler(GPIO_Type* gpio, uint32_t pin) {
    for (int i = 0; i < Gpio::kCount; ++i) {
        if (PinNameToModule[i] == gpio && PinNameToPin[i] == pin) {
            GpioCallback handler = IRQHandlers[i];
            if (handler) {
                handler();
            }
            return;
        }
    }
}

}  // namespace gpio
}  // namespace valiant