/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libs/base/gpio.h"

#include <utility>

#include "libs/base/mutex.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"

#if (__CORTEX_M == 4)
#define GPIO6_Combined_0_15_IRQn NotAvail_IRQn
#endif

namespace coralmicro {
namespace {
StaticSemaphore_t g_mutex_storage;
SemaphoreHandle_t g_mutex;

GPIO_Type* PinNameToModule[Gpio::kCount] = {
    [Gpio::kStatusLed] = GPIO13,   [Gpio::kUserLed] = GPIO13,
    [Gpio::kEdgeTpuPgood] = GPIO8, [Gpio::kEdgeTpuReset] = GPIO8,
    [Gpio::kEdgeTpuPmic] = GPIO8,  [Gpio::kBtRegOn] = GPIO10,
    [Gpio::kUserButton] = GPIO13,  [Gpio::kCameraTrigger] = GPIO8,
    [Gpio::kCameraInt] = GPIO13,   [Gpio::kAntennaSelect] = GPIO11,
    [Gpio::kBtHostWake] = GPIO11,  [Gpio::kBtDevWake] = GPIO11,
    [Gpio::kEthPhyRst] = GPIO8,    [Gpio::kCameraPrivacyOverride] = GPIO8,
    [Gpio::kCryptoRst] = GPIO12,   [Gpio::kLpuart1SwitchEnable] = GPIO9,
    [Gpio::kSpiCs] = GPIO6,        [Gpio::kSpiSck] = GPIO6,
    [Gpio::kSpiSdo] = GPIO6,       [Gpio::kSpiSdi] = GPIO6,
    [Gpio::kSda6] = GPIO6,         [Gpio::kScl1] = GPIO3,
    [Gpio::kSda1] = GPIO4,         [Gpio::kAA] = GPIO3,
    [Gpio::kAB] = GPIO3,           [Gpio::kUartCts] = GPIO2,
    [Gpio::kUartRts] = GPIO2,      [Gpio::kPwm1] = GPIO3,
    [Gpio::kPwm0] = GPIO2,         [Gpio::kScl6] = GPIO6,
};

constexpr uint32_t PinNameToPin[Gpio::kCount] = {
    [Gpio::kStatusLed] = 5,
    [Gpio::kUserLed] = 6,
    [Gpio::kEdgeTpuPgood] = 26,
    [Gpio::kEdgeTpuReset] = 24,
    [Gpio::kEdgeTpuPmic] = 25,
    [Gpio::kBtRegOn] = 2,
    [Gpio::kUserButton] = 3,
    [Gpio::kCameraTrigger] = 27,
    [Gpio::kCameraInt] = 8,
    [Gpio::kAntennaSelect] = 7,
    [Gpio::kBtHostWake] = 16,
    [Gpio::kBtDevWake] = 15,
    [Gpio::kEthPhyRst] = 13,
    [Gpio::kCameraPrivacyOverride] = 22,
    [Gpio::kCryptoRst] = 8,
    [Gpio::kLpuart1SwitchEnable] = 4,
    [Gpio::kSpiCs] = 9,
    [Gpio::kSpiSck] = 10,
    [Gpio::kSpiSdo] = 11,
    [Gpio::kSpiSdi] = 12,
    [Gpio::kSda6] = 6,
    [Gpio::kScl1] = 31,
    [Gpio::kSda1] = 0,
    [Gpio::kAA] = 5,
    [Gpio::kAB] = 6,
    [Gpio::kUartCts] = 10,
    [Gpio::kUartRts] = 11,
    [Gpio::kPwm1] = 0,
    [Gpio::kPwm0] = 31,
    [Gpio::kScl6] = 7,
};

gpio_pin_config_t PinNameToConfig[Gpio::kCount] = {
    [Gpio::kStatusLed] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kUserLed] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kEdgeTpuPgood] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kEdgeTpuReset] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kEdgeTpuPmic] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kBtRegOn] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kUserButton] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_IntFallingEdge,
        },
    [Gpio::kCameraTrigger] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kCameraInt] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_IntRisingEdge,
        },
    [Gpio::kAntennaSelect] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kBtHostWake] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kBtDevWake] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 1,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kEthPhyRst] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kCameraPrivacyOverride] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 1,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kCryptoRst] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 1,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kLpuart1SwitchEnable] =
        {
            .direction = kGPIO_DigitalOutput,
            .outputLogic = 1,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kSpiCs] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kSpiSck] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kSpiSdo] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kSpiSdi] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kSda6] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kScl1] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kSda1] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kAA] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kAB] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kUartCts] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kUartRts] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kPwm1] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kPwm0] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
    [Gpio::kScl6] =
        {
            .direction = kGPIO_DigitalInput,
            .outputLogic = 0,
            .interruptMode = kGPIO_NoIntmode,
        },
};

constexpr IRQn_Type PinNameToIRQ[Gpio::kCount] = {
    [Gpio::kStatusLed] = HardFault_IRQn,
    [Gpio::kUserLed] = HardFault_IRQn,
    [Gpio::kEdgeTpuPgood] = HardFault_IRQn,
    [Gpio::kEdgeTpuReset] = HardFault_IRQn,
    [Gpio::kEdgeTpuPmic] = HardFault_IRQn,
    [Gpio::kBtRegOn] = HardFault_IRQn,
    [Gpio::kUserButton] = GPIO13_Combined_0_31_IRQn,
    [Gpio::kCameraTrigger] = HardFault_IRQn,
    [Gpio::kCameraInt] = GPIO13_Combined_0_31_IRQn,
    [Gpio::kAntennaSelect] = HardFault_IRQn,
    [Gpio::kBtHostWake] = HardFault_IRQn,
    [Gpio::kBtDevWake] = HardFault_IRQn,
    [Gpio::kEthPhyRst] = HardFault_IRQn,
    [Gpio::kCameraPrivacyOverride] = HardFault_IRQn,
    [Gpio::kCryptoRst] = HardFault_IRQn,
    [Gpio::kLpuart1SwitchEnable] = HardFault_IRQn,
    [Gpio::kSpiCs] = GPIO6_Combined_0_15_IRQn,
    [Gpio::kSpiSck] = GPIO6_Combined_0_15_IRQn,
    [Gpio::kSpiSdo] = GPIO6_Combined_0_15_IRQn,
    [Gpio::kSpiSdi] = GPIO6_Combined_0_15_IRQn,
    [Gpio::kSda6] = GPIO6_Combined_0_15_IRQn,
    [Gpio::kScl1] = GPIO3_Combined_16_31_IRQn,
    [Gpio::kSda1] = GPIO4_Combined_0_15_IRQn,
    [Gpio::kAA] = GPIO3_Combined_0_15_IRQn,
    [Gpio::kAB] = GPIO3_Combined_0_15_IRQn,
    [Gpio::kUartCts] = GPIO2_Combined_0_15_IRQn,
    [Gpio::kUartRts] = GPIO2_Combined_0_15_IRQn,
    [Gpio::kPwm1] = GPIO3_Combined_0_15_IRQn,
    [Gpio::kPwm0] = GPIO2_Combined_16_31_IRQn,
    [Gpio::kScl6] = GPIO6_Combined_0_15_IRQn,
};

constexpr uint32_t PinNameToIOMUXC[Gpio::kCount][5] = {
    [Gpio::kStatusLed] = {IOMUXC_GPIO_SNVS_02_DIG_GPIO13_IO05},
    [Gpio::kUserLed] = {IOMUXC_GPIO_SNVS_03_DIG_GPIO13_IO06},
    [Gpio::kEdgeTpuPgood] = {IOMUXC_GPIO_EMC_B2_16_GPIO8_IO26},
    [Gpio::kEdgeTpuReset] = {IOMUXC_GPIO_EMC_B2_14_GPIO8_IO24},
    [Gpio::kEdgeTpuPmic] = {IOMUXC_GPIO_EMC_B2_15_GPIO8_IO25},
    [Gpio::kBtRegOn] = {IOMUXC_GPIO_AD_35_GPIO10_IO02},
    [Gpio::kUserButton] = {IOMUXC_GPIO_SNVS_00_DIG_GPIO13_IO03},
    [Gpio::kCameraTrigger] = {IOMUXC_GPIO_EMC_B2_17_GPIO8_IO27},
    [Gpio::kCameraInt] = {IOMUXC_GPIO_SNVS_05_DIG_GPIO13_IO08},
    [Gpio::kAntennaSelect] = {IOMUXC_GPIO_DISP_B2_06_GPIO11_IO07},
    [Gpio::kBtHostWake] = {IOMUXC_GPIO_DISP_B2_15_GPIO11_IO16},
    [Gpio::kBtDevWake] = {IOMUXC_GPIO_DISP_B2_14_GPIO11_IO15},
    [Gpio::kEthPhyRst] = {IOMUXC_GPIO_EMC_B2_03_GPIO8_IO13},
    [Gpio::kCameraPrivacyOverride] = {IOMUXC_GPIO_EMC_B2_12_GPIO8_IO22},
    [Gpio::kCryptoRst] = {IOMUXC_GPIO_LPSR_08_GPIO12_IO08},
    [Gpio::kLpuart1SwitchEnable] = {IOMUXC_GPIO_AD_05_GPIO9_IO04},
    [Gpio::kSpiCs] = {IOMUXC_GPIO_LPSR_09_GPIO_MUX6_IO09},
    [Gpio::kSpiSck] = {IOMUXC_GPIO_LPSR_10_GPIO_MUX6_IO10},
    [Gpio::kSpiSdo] = {IOMUXC_GPIO_LPSR_11_GPIO_MUX6_IO11},
    [Gpio::kSpiSdi] = {IOMUXC_GPIO_LPSR_12_GPIO_MUX6_IO12},
    [Gpio::kSda6] = {IOMUXC_GPIO_LPSR_06_GPIO_MUX6_IO06},
    [Gpio::kScl1] = {IOMUXC_GPIO_AD_32_GPIO_MUX3_IO31},
    [Gpio::kSda1] = {IOMUXC_GPIO_AD_33_GPIO_MUX4_IO00},
    [Gpio::kAA] = {IOMUXC_GPIO_AD_06_GPIO_MUX3_IO05},
    [Gpio::kAB] = {IOMUXC_GPIO_AD_07_GPIO_MUX3_IO06},
    [Gpio::kUartCts] = {IOMUXC_GPIO_EMC_B2_00_GPIO_MUX2_IO10},
    [Gpio::kUartRts] = {IOMUXC_GPIO_EMC_B2_01_GPIO_MUX2_IO11},
    [Gpio::kPwm1] = {IOMUXC_GPIO_AD_01_GPIO_MUX3_IO00},
    [Gpio::kPwm0] = {IOMUXC_GPIO_AD_00_GPIO_MUX2_IO31},
    [Gpio::kScl6] = {IOMUXC_GPIO_LPSR_07_GPIO_MUX6_IO07},
};

constexpr uint32_t PinNameToPullMask[Gpio::kCount] = {
    [Gpio::kStatusLed] = 0x0000000C,
    [Gpio::kUserLed] = 0x0000000C,
    [Gpio::kEdgeTpuPgood] = 0x0000000C,
    [Gpio::kEdgeTpuReset] = 0x0000000C,
    [Gpio::kEdgeTpuPmic] = 0x0000000C,
    [Gpio::kBtRegOn] = 0x0000000C,
    [Gpio::kUserButton] = 0x0000000C,
    [Gpio::kCameraTrigger] = 0x0000000C,
    [Gpio::kCameraInt] = 0x0000000C,
    [Gpio::kAntennaSelect] = 0x0000000C,
    [Gpio::kBtHostWake] = 0x0000000C,
    [Gpio::kBtDevWake] = 0x0000000C,
    [Gpio::kEthPhyRst] = 0x0000000C,
    [Gpio::kCameraPrivacyOverride] = 0x0000000C,
    [Gpio::kCryptoRst] = 0x0000000C,
    [Gpio::kLpuart1SwitchEnable] = 0x0000000C,
    [Gpio::kSpiCs] = 0x0000000C,
    [Gpio::kSpiSck] = 0x0000000C,
    [Gpio::kSpiSdo] = 0x0000000C,
    [Gpio::kSpiSdi] = 0x0000000C,
    [Gpio::kSda6] = 0x0000000C,
    [Gpio::kScl1] = 0x0000000C,
    [Gpio::kSda1] = 0x0000000C,
    [Gpio::kAA] = 0x0000000C,
    [Gpio::kAB] = 0x0000000C,
    [Gpio::kUartCts] = 0x0000000C,
    [Gpio::kUartRts] = 0x0000000C,
    [Gpio::kPwm1] = 0x0000000C,
    [Gpio::kPwm0] = 0x0000000C,
    [Gpio::kScl6] = 0x0000000C,
};

constexpr uint32_t PinNameToNoPull[Gpio::kCount] = {
    [Gpio::kStatusLed] = 0x0000000C,
    [Gpio::kUserLed] = 0x0000000C,
    [Gpio::kEdgeTpuPgood] = 0x0000000C,
    [Gpio::kEdgeTpuReset] = 0x0000000C,
    [Gpio::kEdgeTpuPmic] = 0x0000000C,
    [Gpio::kBtRegOn] = 0x00000000,
    [Gpio::kUserButton] = 0x00000000,
    [Gpio::kCameraTrigger] = 0x0000000C,
    [Gpio::kCameraInt] = 0x00000000,
    [Gpio::kAntennaSelect] = 0x00000000,
    [Gpio::kBtHostWake] = 0x00000000,
    [Gpio::kBtDevWake] = 0x00000000,
    [Gpio::kEthPhyRst] = 0x0000000C,
    [Gpio::kCameraPrivacyOverride] = 0x0000000C,
    [Gpio::kCryptoRst] = 0x00000000,
    [Gpio::kLpuart1SwitchEnable] = 0x00000000,
    [Gpio::kSpiCs] = 0x00000000,
    [Gpio::kSpiSck] = 0x00000000,
    [Gpio::kSpiSdo] = 0x00000000,
    [Gpio::kSpiSdi] = 0x00000000,
    [Gpio::kSda6] = 0x00000000,
    [Gpio::kScl1] = 0x00000000,
    [Gpio::kSda1] = 0x00000000,
    [Gpio::kAA] = 0x00000000,
    [Gpio::kAB] = 0x00000000,
    [Gpio::kUartCts] = 0x0000000C,
    [Gpio::kUartRts] = 0x0000000C,
    [Gpio::kPwm1] = 0x00000000,
    [Gpio::kPwm0] = 0x00000000,
    [Gpio::kScl6] = 0x00000000,
};

constexpr uint32_t PinNameToPullUp[Gpio::kCount] = {
    [Gpio::kStatusLed] = 0x0000000C,
    [Gpio::kUserLed] = 0x0000000C,
    [Gpio::kEdgeTpuPgood] = 0x00000004,
    [Gpio::kEdgeTpuReset] = 0x00000004,
    [Gpio::kEdgeTpuPmic] = 0x00000004,
    [Gpio::kBtRegOn] = 0x0000000C,
    [Gpio::kUserButton] = 0x0000000C,
    [Gpio::kCameraTrigger] = 0x00000004,
    [Gpio::kCameraInt] = 0x0000000C,
    [Gpio::kAntennaSelect] = 0x0000000C,
    [Gpio::kBtHostWake] = 0x0000000C,
    [Gpio::kBtDevWake] = 0x0000000C,
    [Gpio::kEthPhyRst] = 0x00000004,
    [Gpio::kCameraPrivacyOverride] = 0x00000004,
    [Gpio::kCryptoRst] = 0x0000000C,
    [Gpio::kLpuart1SwitchEnable] = 0x0000000C,
    [Gpio::kSpiCs] = 0x00000000,
    [Gpio::kSpiSck] = 0x00000000,
    [Gpio::kSpiSdo] = 0x00000000,
    [Gpio::kSpiSdi] = 0x00000000,
    [Gpio::kSda6] = 0x0000000C,
    [Gpio::kScl1] = 0x0000000C,
    [Gpio::kSda1] = 0x0000000C,
    [Gpio::kAA] = 0x0000000C,
    [Gpio::kAB] = 0x0000000C,
    [Gpio::kUartCts] = 0x00000004,
    [Gpio::kUartRts] = 0x00000004,
    [Gpio::kPwm1] = 0x0000000C,
    [Gpio::kPwm0] = 0x0000000C,
    [Gpio::kScl6] = 0x0000000C,
};

constexpr uint32_t PinNameToPullDown[Gpio::kCount] = {
    [Gpio::kStatusLed] = 0x00000004,
    [Gpio::kUserLed] = 0x00000004,
    [Gpio::kEdgeTpuPgood] = 0x00000008,
    [Gpio::kEdgeTpuReset] = 0x00000008,
    [Gpio::kEdgeTpuPmic] = 0x00000008,
    [Gpio::kBtRegOn] = 0x00000008,
    [Gpio::kUserButton] = 0x00000004,
    [Gpio::kCameraTrigger] = 0x00000008,
    [Gpio::kCameraInt] = 0x00000004,
    [Gpio::kAntennaSelect] = 0x00000004,
    [Gpio::kBtHostWake] = 0x00000008,
    [Gpio::kBtDevWake] = 0x00000008,
    [Gpio::kEthPhyRst] = 0x00000008,
    [Gpio::kCameraPrivacyOverride] = 0x00000008,
    [Gpio::kCryptoRst] = 0x00000004,
    [Gpio::kLpuart1SwitchEnable] = 0x00000004,
    [Gpio::kSpiCs] = 0x00000004,
    [Gpio::kSpiSck] = 0x00000004,
    [Gpio::kSpiSdo] = 0x00000004,
    [Gpio::kSpiSdi] = 0x00000004,
    [Gpio::kSda6] = 0x00000004,
    [Gpio::kScl1] = 0x00000004,
    [Gpio::kSda1] = 0x00000004,
    [Gpio::kAA] = 0x00000004,
    [Gpio::kAB] = 0x00000004,
    [Gpio::kUartCts] = 0x00000008,
    [Gpio::kUartRts] = 0x00000008,
    [Gpio::kPwm1] = 0x00000004,
    [Gpio::kPwm0] = 0x00000004,
    [Gpio::kScl6] = 0x00000004,
};

constexpr gpio_interrupt_mode_t
    InterruptModeToGpioIntMode[GpioInterruptMode::kIntModeCount] = {
        [GpioInterruptMode::kIntModeNone] = kGPIO_NoIntmode,
        [GpioInterruptMode::kIntModeLow] = kGPIO_IntLowLevel,
        [GpioInterruptMode::kIntModeHigh] = kGPIO_IntHighLevel,
        [GpioInterruptMode::kIntModeRising] = kGPIO_IntRisingEdge,
        [GpioInterruptMode::kIntModeFalling] = kGPIO_IntFallingEdge,
        [GpioInterruptMode::kIntModeChanging] = kGPIO_IntRisingOrFallingEdge,
};

GpioCallback g_irq_handlers[Gpio::kCount];
uint64_t g_irq_debounce_us[Gpio::kCount];
uint64_t g_irq_last_time_us[Gpio::kCount];

void IrqHandler(GPIO_Type* gpio, uint32_t pin) {
  for (int i = 0; i < Gpio::kCount; ++i) {
    if (PinNameToModule[i] == gpio && PinNameToPin[i] == pin) {
      auto& handler = g_irq_handlers[i];
      auto now = TimerMicros();
      if (g_irq_debounce_us[i] == 0 ||
          (g_irq_debounce_us[i] &&
           ((now - g_irq_last_time_us[i]) >= g_irq_debounce_us[i]))) {
        if (handler) {
          handler();
        }
        g_irq_last_time_us[i] = now;
        return;
      }
    }
  }
}

void CommonIrqHandler(GPIO_Type* base) {
  uint32_t pins = GPIO_PortGetInterruptFlags(base);
  GPIO_PortClearInterruptFlags(base, pins);
  int i = 0;
  while (pins) {
    if (pins & 1) {
      IrqHandler(base, i);
    }
    ++i;
    pins = pins >> 1;
  }
}
}  // namespace

void GpioInit() {
  g_mutex = xSemaphoreCreateMutexStatic(&g_mutex_storage);

  for (int i = 0; i < Gpio::kCount; ++i) {
    g_irq_handlers[i] = nullptr;
    g_irq_debounce_us[i] = 0;
    g_irq_last_time_us[i] = 0;
    switch (i) {
      case Gpio::kEdgeTpuPgood:
      case Gpio::kEdgeTpuReset:
      case Gpio::kEdgeTpuPmic:
      case Gpio::kBtHostWake:
      case Gpio::kBtDevWake:
      case Gpio::kEthPhyRst:
#if (__CORTEX_M == 4)
        break;  // Do not initialize tpu or ethernet gpios for the m4.
#endif
      default:
        GPIO_PinInit(PinNameToModule[i], PinNameToPin[i], &PinNameToConfig[i]);
    }
    if (PinNameToConfig[i].interruptMode != kGPIO_NoIntmode) {
      GPIO_PinSetInterruptConfig(PinNameToModule[i], PinNameToPin[i],
                                 PinNameToConfig[i].interruptMode);
      GPIO_PortClearInterruptFlags(
          PinNameToModule[i], GPIO_PortGetInterruptFlags(PinNameToModule[i]));
      GPIO_PortEnableInterrupts(PinNameToModule[i], 1 << PinNameToPin[i]);
      EnableIRQ(PinNameToIRQ[i]);
      NVIC_SetPriority(PinNameToIRQ[i], 5);
    }
  }
}

void GpioSet(Gpio gpio, bool enable) {
  MutexLock lock(g_mutex);
  GPIO_PinWrite(PinNameToModule[gpio], PinNameToPin[gpio], enable);
}

bool GpioGet(Gpio gpio) {
  MutexLock lock(g_mutex);
  return GPIO_PinRead(PinNameToModule[gpio], PinNameToPin[gpio]);
}

void GpioSetMode(Gpio gpio, GpioMode mode) {
  auto* config = &PinNameToConfig[gpio];
  config->direction =
      mode == GpioMode::kOutput ? kGPIO_DigitalOutput : kGPIO_DigitalInput;
  GPIO_PinInit(PinNameToModule[gpio], PinNameToPin[gpio], config);
  auto iomuxc = PinNameToIOMUXC[gpio];
  uint32_t pin_config = *((volatile uint32_t*)iomuxc[4]);
  pin_config &= ~PinNameToPullMask[gpio];
  if (mode == GpioMode::kInputPullUp) {
    pin_config |= PinNameToPullUp[gpio];
  } else if (mode == GpioMode::kInputPullDown) {
    pin_config |= PinNameToPullDown[gpio];
  } else {
    pin_config |= PinNameToNoPull[gpio];
  }
  IOMUXC_SetPinMux(iomuxc[0], iomuxc[1], iomuxc[2], iomuxc[3], iomuxc[4], 1U);
  IOMUXC_SetPinConfig(iomuxc[0], iomuxc[1], iomuxc[2], iomuxc[3], iomuxc[4],
                      pin_config);
}

void GpioConfigureInterrupt(Gpio gpio, GpioInterruptMode mode,
                            GpioCallback cb) {
  GpioConfigureInterrupt(gpio, mode, cb, 0);
}

void GpioConfigureInterrupt(Gpio gpio, GpioInterruptMode mode, GpioCallback cb,
                            uint64_t debounce_interval_us) {
  auto* config = &PinNameToConfig[gpio];
  config->direction = kGPIO_DigitalInput;
  config->interruptMode = InterruptModeToGpioIntMode[mode];
  GPIO_PinInit(PinNameToModule[gpio], PinNameToPin[gpio], config);
  if (PinNameToConfig[gpio].interruptMode != kGPIO_NoIntmode) {
    g_irq_handlers[gpio] = std::move(cb);
    g_irq_debounce_us[gpio] = debounce_interval_us;
    g_irq_last_time_us[gpio] = TimerMicros();
    GPIO_PinSetInterruptConfig(PinNameToModule[gpio], PinNameToPin[gpio],
                               PinNameToConfig[gpio].interruptMode);
    GPIO_PortClearInterruptFlags(
        PinNameToModule[gpio],
        GPIO_PortGetInterruptFlags(PinNameToModule[gpio]));
    GPIO_PortEnableInterrupts(PinNameToModule[gpio], 1 << PinNameToPin[gpio]);
    EnableIRQ(PinNameToIRQ[gpio]);
    NVIC_SetPriority(PinNameToIRQ[gpio], 5);
  } else {
    DisableIRQ(PinNameToIRQ[gpio]);
    g_irq_handlers[gpio] = []() {};
    g_irq_debounce_us[gpio] = 0;
    g_irq_last_time_us[gpio] = 0;
  }
}

void GpioRegisterIrqHandler(Gpio gpio, GpioCallback cb) {}
}  // namespace coralmicro

#define GPIO_IRQHandler(base, irq)      \
  extern "C" void irq() {               \
    coralmicro::CommonIrqHandler(base); \
    SDK_ISR_EXIT_BARRIER;               \
  }

GPIO_IRQHandler(GPIO2, GPIO2_Combined_0_15_IRQHandler);
GPIO_IRQHandler(GPIO2, GPIO2_Combined_16_31_IRQHandler);
GPIO_IRQHandler(GPIO3, GPIO3_Combined_16_31_IRQHandler);
GPIO_IRQHandler(GPIO4, GPIO4_Combined_0_15_IRQHandler);
GPIO_IRQHandler(GPIO6, GPIO6_Combined_0_15_IRQHandler);
GPIO_IRQHandler(GPIO13, GPIO13_Combined_0_31_IRQHandler);
