#include "libs/base/mutex.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"

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

#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
extern "C" void GPIO6_Combined_0_15_IRQHandler(void) {
    uint32_t pins = GPIO_PortGetInterruptFlags(GPIO6);
    GPIO_PortClearInterruptFlags(GPIO6, pins);
    int i = 0;
    while (pins) {
        if (pins & 1) {
            valiant::gpio::IRQHandler(GPIO6, i);
        }
        ++i;
        pins = pins >> 1;
    }
    SDK_ISR_EXIT_BARRIER;
}

extern "C" void GPIO2_Combined_0_15_IRQHandler(void) {
    uint32_t pins = GPIO_PortGetInterruptFlags(GPIO2);
    GPIO_PortClearInterruptFlags(GPIO2, pins);
    int i = 0;
    while (pins) {
        if (pins & 1) {
            valiant::gpio::IRQHandler(GPIO2, i);
        }
        ++i;
        pins = pins >> 1;
    }
    SDK_ISR_EXIT_BARRIER;
}
#endif

namespace valiant {
namespace gpio {

static SemaphoreHandle_t gpio_semaphore;
static StaticSemaphore_t gpio_semaphore_static;
static GPIO_Type* PinNameToModule[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = GPIO6,
    [Gpio::kArduinoD1] = GPIO2,
    [Gpio::kArduinoD2] = GPIO2,
    [Gpio::kArduinoD3] = GPIO6,
#endif
    [Gpio::kPowerLED] = GPIO13,
    [Gpio::kUserLED] = GPIO13,
    [Gpio::kTpuLED] = GPIO9,
    [Gpio::kEdgeTpuPgood] = GPIO8,
    [Gpio::kEdgeTpuReset] = GPIO8,
    [Gpio::kEdgeTpuPmic] = GPIO8,
    [Gpio::kBtRegOn] = GPIO10,
    [Gpio::kUserButton] = GPIO13,
    [Gpio::kCameraTrigger] = GPIO8,
    [Gpio::kAntennaSelect] = GPIO11,
    [Gpio::kBtHostWake] = GPIO11,
    [Gpio::kBtDevWake] = GPIO11,
    [Gpio::kEthPhyRst] = GPIO8,
    [Gpio::kBufferEnable] = GPIO9,
    [Gpio::kCameraPrivacyOverride] = GPIO8,
};

static uint32_t PinNameToPin[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = 7,
    [Gpio::kArduinoD1] = 11,
    [Gpio::kArduinoD2] = 10,
    [Gpio::kArduinoD3] = 6,
#endif
    [Gpio::kPowerLED] = 5,
    [Gpio::kUserLED] = 6,
    [Gpio::kTpuLED] = 1,
    [Gpio::kEdgeTpuPgood] = 26,
    [Gpio::kEdgeTpuReset] = 24,
    [Gpio::kEdgeTpuPmic] = 25,
    [Gpio::kBtRegOn] = 2,
    [Gpio::kUserButton] = 3,
    [Gpio::kCameraTrigger] = 27,
    [Gpio::kAntennaSelect] = 7,
    [Gpio::kBtHostWake] = 16,
    [Gpio::kBtDevWake] = 15,
    [Gpio::kEthPhyRst] = 13,
    [Gpio::kBufferEnable] = 4,
    [Gpio::kCameraPrivacyOverride] = 22,
};

static gpio_pin_config_t PinNameToConfig[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_IntFallingEdge,
    },
    [Gpio::kArduinoD1] = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_IntFallingEdge,
    },
    [Gpio::kArduinoD2] = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_IntFallingEdge,
    },
    [Gpio::kArduinoD3] = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_IntFallingEdge,
    },
#endif
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
    [Gpio::kTpuLED] = {
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
    [Gpio::kBtHostWake] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kBtDevWake] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 1,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kEthPhyRst] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kBufferEnable] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 1,
        .interruptMode = kGPIO_NoIntmode,
    },
    [Gpio::kCameraPrivacyOverride] = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 1,
        .interruptMode = kGPIO_NoIntmode,
    },
};

static IRQn_Type PinNameToIRQ[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = GPIO6_Combined_0_15_IRQn,
    [Gpio::kArduinoD1] = GPIO2_Combined_0_15_IRQn,
    [Gpio::kArduinoD2] = GPIO2_Combined_0_15_IRQn,
    [Gpio::kArduinoD3] = GPIO6_Combined_0_15_IRQn,
#endif
    [Gpio::kPowerLED] = HardFault_IRQn,
    [Gpio::kUserLED] = HardFault_IRQn,
    [Gpio::kTpuLED] = HardFault_IRQn,
    [Gpio::kEdgeTpuPgood] = HardFault_IRQn,
    [Gpio::kEdgeTpuReset] = HardFault_IRQn,
    [Gpio::kEdgeTpuPmic] = HardFault_IRQn,
    [Gpio::kBtRegOn] = HardFault_IRQn,
    [Gpio::kUserButton] = GPIO13_Combined_0_31_IRQn,
    [Gpio::kCameraTrigger] = HardFault_IRQn,
    [Gpio::kAntennaSelect] = HardFault_IRQn,
    [Gpio::kBtHostWake] = HardFault_IRQn,
    [Gpio::kBtDevWake] = HardFault_IRQn,
    [Gpio::kEthPhyRst] = HardFault_IRQn,
    [Gpio::kBufferEnable] = HardFault_IRQn,
    [Gpio::kCameraPrivacyOverride] = HardFault_IRQn,
};

static uint32_t PinNameToIOMUXC[Gpio::kCount][5] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = {IOMUXC_GPIO_LPSR_07_GPIO_MUX6_IO07},
    [Gpio::kArduinoD1] = {IOMUXC_GPIO_EMC_B2_01_GPIO_MUX2_IO11},
    [Gpio::kArduinoD2] = {IOMUXC_GPIO_EMC_B2_00_GPIO_MUX2_IO10},
    [Gpio::kArduinoD3] = {IOMUXC_GPIO_LPSR_06_GPIO_MUX6_IO06},
#endif
    [Gpio::kPowerLED] = {IOMUXC_GPIO_SNVS_02_DIG_GPIO13_IO05},
    [Gpio::kUserLED] = {IOMUXC_GPIO_SNVS_03_DIG_GPIO13_IO06},
    [Gpio::kTpuLED] = {IOMUXC_GPIO_AD_02_GPIO9_IO01},
    [Gpio::kEdgeTpuPgood] = {IOMUXC_GPIO_EMC_B2_16_GPIO8_IO26},
    [Gpio::kEdgeTpuReset] = {IOMUXC_GPIO_EMC_B2_14_GPIO8_IO24},
    [Gpio::kEdgeTpuPmic] = {IOMUXC_GPIO_EMC_B2_15_GPIO8_IO25},
    [Gpio::kBtRegOn] = {IOMUXC_GPIO_AD_35_GPIO10_IO02},
    [Gpio::kUserButton] = {IOMUXC_GPIO_SNVS_00_DIG_GPIO13_IO03},
    [Gpio::kCameraTrigger] = {IOMUXC_GPIO_EMC_B2_17_GPIO8_IO27},
    [Gpio::kAntennaSelect] = {IOMUXC_GPIO_DISP_B2_06_GPIO11_IO07},
};

static uint32_t PinNameToPullMask[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = 0x0000000C,
    [Gpio::kArduinoD1] = 0x0000000C,
    [Gpio::kArduinoD2] = 0x0000000C,
    [Gpio::kArduinoD3] = 0x0000000C,
#endif
    [Gpio::kPowerLED] = 0x0000000C,
    [Gpio::kUserLED] = 0x0000000C,
    [Gpio::kTpuLED] = 0x0000000C,
    [Gpio::kEdgeTpuPgood] = 0x0000000C,
    [Gpio::kEdgeTpuReset] = 0x0000000C,
    [Gpio::kEdgeTpuPmic] = 0x0000000C,
    [Gpio::kBtRegOn] = 0x0000000C,
    [Gpio::kUserButton] = 0x0000000C,
    [Gpio::kCameraTrigger] = 0x0000000C,
    [Gpio::kAntennaSelect] = 0x0000000C,
};

static uint32_t PinNameToNoPull[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = 0x00000000,
    [Gpio::kArduinoD1] = 0x0000000C,
    [Gpio::kArduinoD2] = 0x0000000C,
    [Gpio::kArduinoD3] = 0x00000000,
#endif
    [Gpio::kPowerLED] = 0x0000000C,
    [Gpio::kUserLED] = 0x0000000C,
    [Gpio::kTpuLED] = 0x00000000,
    [Gpio::kEdgeTpuPgood] = 0x0000000C,
    [Gpio::kEdgeTpuReset] = 0x0000000C,
    [Gpio::kEdgeTpuPmic] = 0x0000000C,
    [Gpio::kBtRegOn] = 0x00000000,
    [Gpio::kUserButton] = 0x00000000,
    [Gpio::kCameraTrigger] = 0x0000000C,
    [Gpio::kAntennaSelect] = 0x00000000,
};

static uint32_t PinNameToPullUp[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = 0x0000000C,
    [Gpio::kArduinoD1] = 0x00000004,
    [Gpio::kArduinoD2] = 0x00000004,
    [Gpio::kArduinoD3] = 0x0000000C,
#endif
    [Gpio::kPowerLED] = 0x0000000C,
    [Gpio::kUserLED] = 0x0000000C,
    [Gpio::kTpuLED] = 0x0000000C,
    [Gpio::kEdgeTpuPgood] = 0x00000004,
    [Gpio::kEdgeTpuReset] = 0x00000004,
    [Gpio::kEdgeTpuPmic] = 0x00000004,
    [Gpio::kBtRegOn] = 0x0000000C,
    [Gpio::kUserButton] = 0x0000000C,
    [Gpio::kCameraTrigger] = 0x00000004,
    [Gpio::kAntennaSelect] = 0x0000000C,
};

static uint32_t PinNameToPullDown[Gpio::kCount] = {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    [Gpio::kArduinoD0] = 0x00000004,
    [Gpio::kArduinoD1] = 0x00000008,
    [Gpio::kArduinoD2] = 0x00000008,
    [Gpio::kArduinoD3] = 0x00000004,
#endif
    [Gpio::kPowerLED] = 0x00000004,
    [Gpio::kUserLED] = 0x00000004,
    [Gpio::kTpuLED] = 0x00000008,
    [Gpio::kEdgeTpuPgood] = 0x00000008,
    [Gpio::kEdgeTpuReset] = 0x00000008,
    [Gpio::kEdgeTpuPmic] = 0x00000008,
    [Gpio::kBtRegOn] = 0x00000008,
    [Gpio::kUserButton] = 0x00000004,
    [Gpio::kCameraTrigger] = 0x00000008,
    [Gpio::kAntennaSelect] = 0x00000004,
};

static gpio_interrupt_mode_t InterruptModeToGpioIntMode[InterruptMode::kIntModeCount] = {
    [InterruptMode::kIntModeNone] = kGPIO_NoIntmode,
    [InterruptMode::kIntModeLow] = kGPIO_IntLowLevel,
    [InterruptMode::kIntModeHigh] = kGPIO_IntHighLevel,
    [InterruptMode::kIntModeRising] = kGPIO_IntRisingEdge,
    [InterruptMode::kIntModeFalling] = kGPIO_IntFallingEdge,
    [InterruptMode::kIntModeChanging] = kGPIO_IntRisingOrFallingEdge,
};

static GpioCallback IRQHandlers[Gpio::kCount];

void Init() {
    gpio_semaphore = xSemaphoreCreateMutexStatic(&gpio_semaphore_static);

    bool m4{false};
#if (__CORTEX_M == 4)
    m4 = true;
#endif
    for (int i = 0; i < Gpio::kCount; ++i) {
        IRQHandlers[i] = nullptr;
        switch (i) {
            case Gpio::kTpuLED:
            case Gpio::kEdgeTpuPgood:
            case Gpio::kEdgeTpuReset:
            case Gpio::kEdgeTpuPmic:
                if (m4)  break; // Do not initialize tpu gpios for the m4.
            default:
               GPIO_PinInit(
                PinNameToModule[i],
                PinNameToPin[i],
                &PinNameToConfig[i]);
        }
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

void SetMode(Gpio gpio, bool input, bool pull, bool pull_direction) {
    auto *config = &PinNameToConfig[gpio];
    config->direction = input ? kGPIO_DigitalInput : kGPIO_DigitalOutput;
    GPIO_PinInit(PinNameToModule[gpio], PinNameToPin[gpio], config);
    auto iomuxc = PinNameToIOMUXC[gpio];
    uint32_t pin_config = *((volatile uint32_t*)iomuxc[4]);
    pin_config &= ~PinNameToPullMask[gpio];
    if (pull) {
        if (pull_direction) {// up
            pin_config |= PinNameToPullUp[gpio];
        } else {
            pin_config |= PinNameToPullDown[gpio];
        }
    } else {
        pin_config |= PinNameToNoPull[gpio];
    }
    IOMUXC_SetPinConfig(iomuxc[0], iomuxc[1], iomuxc[2], iomuxc[3], iomuxc[4], pin_config);
}

void SetIntMode(Gpio gpio, InterruptMode mode) {
    auto *config = &PinNameToConfig[gpio];
    config->direction = kGPIO_DigitalInput;
    config->interruptMode = InterruptModeToGpioIntMode[mode];
    GPIO_PinInit(PinNameToModule[gpio], PinNameToPin[gpio], config);
    if (PinNameToConfig[gpio].interruptMode != kGPIO_NoIntmode) {
        GPIO_PinSetInterruptConfig(PinNameToModule[gpio], PinNameToPin[gpio], PinNameToConfig[gpio].interruptMode);
        GPIO_PortClearInterruptFlags(PinNameToModule[gpio], GPIO_PortGetInterruptFlags(PinNameToModule[gpio]));
        GPIO_PortEnableInterrupts(PinNameToModule[gpio], 1 << PinNameToPin[gpio]);
        EnableIRQ(PinNameToIRQ[gpio]);
        NVIC_SetPriority(PinNameToIRQ[gpio], 5);
    } else {
        DisableIRQ(PinNameToIRQ[gpio]);
    }
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

gpio_pin_config_t GetPinConfig(Gpio gpio) {
    return PinNameToConfig[gpio];
}

void SetPinConfig(Gpio gpio, gpio_pin_config_t config) {
    GPIO_PinInit(PinNameToModule[gpio], PinNameToPin[gpio], &config);
}

}  // namespace gpio
}  // namespace valiant
