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

#ifndef LIBS_BASE_GPIO_H_
#define LIBS_BASE_GPIO_H_

#include <functional>

namespace coralmicro {

// Pre-configured GPIO pins.
enum Gpio {
  // On-board Status LED (orange). Instead use `LedSet()`.
  kStatusLed,
  // On-board User LED (green). Instead use `LedSet()`.
  kUserLed,
  // Power-good signal from TPU. Active high.
  kEdgeTpuPgood,
  // Reset input to TPU. Active low.
  kEdgeTpuReset,
  // Power-enable input to TPU. Active high.
  kEdgeTpuPmic,
  // Enables power regulator on CYW43455 (via B2B connector)
  kBtRegOn,
  // On-board User button. Active low.
  kUserButton,
  // Trigger GPIO for single-shot camera capture.
  kCameraTrigger,
  // Selects between on-board antenna and antenna connector (for Wi-Fi and BT).
  // Low for internal, high for external.
  kAntennaSelect,
  // Input from Bluetooth to wake the host, if it was sleeping.
  kBtHostWake,
  // Output to Bluetooth, to wake the Bluetooth module from sleep.
  kBtDevWake,
  // Reset signal to the Ethernet PHY. Active low.
  kEthPhyRst,
  // Override for the Camera LED. Low to disable the LED.
  kCameraPrivacyOverride,
  // Reset signal to the A71CH. Active low.
  kCryptoRst,
  // SPI6_CS (GPIO_LPSR_09); right header (J10); pin 5
  kSpiCs,
  // SPI6_SCK (GPIO_LPSR_10); right header (J10); pin 6
  kSpiSck,
  // SPI6_SDO (GPIO_LPSR_11); right header (J10); pin 7
  kSpiSdo,
  // SPI6_SDI (GPIO_LPSR_12); right header (J10); pin 8
  kSpiSdi,
  // I2C6_SDA (GPIO_LPSR_06); right header (J10); pin 10
  kSda6,
  // I2C1_SCL (GPIO_AD_32); right header (J10); pin 11
  kScl1,
  // I2C1_SDA (GPIO_AD_33); right header (J10); pin 12
  kSda1,
  // ADC1_CH0A (GPIO_AD_06); left header (J9), pin 3
  kAA,
  // ADC1_CH0B (GPIO_AD_07); left header (J9), pin 4
  kAB,
  // UART6_CTS (GPIO_EMC_B2_00); left header (J9), pin 7
  kUartCts,
  // UART6_RTS (GPIO_EMC_B2_01); left header (J9), pin 8
  kUartRts,
  // PWM_B (GPIO_AD_01); left header (J9), pin 9
  kPwm1,
  // PWM_A (GPIO_AD_00); left header (J9), pin 10
  kPwm0,
  // I2C6_SCL (GPIO_LPSR_07); left header (J9), pin 11
  kScl6,
  kCount,

  kArduinoD0 = kScl6,
  kArduinoD1 = kUartRts,
  kArduinoD2 = kUartCts,
  kArduinoD3 = kSda6,
};

// Enumeration of available interrupt modes for GPIOs.
enum GpioInterruptMode {
  kIntModeNone,
  kIntModeLow,
  kIntModeHigh,
  kIntModeRising,
  kIntModeFalling,
  kIntModeChanging,
  kIntModeCount
};

// The function type required by `RegisterIRQHandler()`.
using GpioCallback = std::function<void()>;

// @cond Do not generate docs
void GpioInit();
// @endcond

// Sets the output value of a GPIO.
// @param gpio Pin to configure. Only pins in the `Gpio` enumeration can be
// configured with this module.
//             To use a GPIO that is not covered by this module,
//             use the functions in
//             `third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h`
// @param enable Whether to set the pin to high or low.
void GpioSet(Gpio gpio, bool enable);

// Gets the input value of a GPIO.
// @param gpio Pin to read.
// @returns Boolean representing high or low state of the pin.
bool GpioGet(Gpio gpio);

// Sets the mode of a GPIO.
// @param gpio Pin to configure.
// @param input True sets the pin as an input; false sets it as an output.
// @param pull True enables the pin to have a pull; false disables it.
// @param pull_direction True enables pull up; false enables pull down. This is
// ignored if `pull` is false.
void GpioSetMode(Gpio gpio, bool input, bool pull, bool pull_direction);

// Sets the interrupt mode of a GPIO.
// @param gpio Pin to configure.
// @param mode The style of interrupt to sense.
void GpioSetIntMode(Gpio gpio, GpioInterruptMode mode);

// Register an interrupt handler for a GPIO.
//
// **Example** (from `examples/button_led/`):
//
// \snippet button_led/button_led.cc gpio-callback
//
// @param Pin for which to register the handler.
// @param cb Callback function that will be invoked when the interrupt is
// raised.
//           This is called from interrupt context, so it should not do much
//           work.
void GpioRegisterIrqHandler(Gpio gpio, GpioCallback cb);

}  // namespace coralmicro

#endif  // LIBS_BASE_GPIO_H_
