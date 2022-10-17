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
  // Trigger GPIO for single-shot camera capture. Not to be used by apps;
  // instead see `CameraTask::Trigger()`.
  kCameraTrigger,
  // Input from the camera to indicate motion was detected.
  kCameraInt,
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
  // Enable signal for the switch that blocks LPUART1 during boot.
  // Set high to allow the LPUART1 signal to pass through.
  kLpuart1SwitchEnable,
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
  // Number of pre-configured GPIOs
  kCount,

  // @cond Do not generate docs
  kArduinoD0 = kScl6,
  kArduinoD1 = kUartRts,
  kArduinoD2 = kUartCts,
  kArduinoD3 = kSda6,
  kArduinoA0 = kAB,
  kArduinoA1 = kAA,
  kArduinoA3 = kPwm0,
  kArduinoA4 = kPwm1,
  // @endcond
};

// Interrupt modes for use with `GpioConfigureInterrupt()`.
enum GpioInterruptMode {
  // Disables GPIO interrupt
  kIntModeNone,
  // Interrupt when line is low
  kIntModeLow,
  // Interrupt when line is high
  kIntModeHigh,
  // Interrupt when line is rising
  kIntModeRising,
  // Interrupt when line is falling
  kIntModeFalling,
  // Interrupt when line is either rising or falling
  kIntModeChanging,
  // Number of interrupt modes
  kIntModeCount
};

// GPIO modes for `GpioSetMode()`.
enum class GpioMode { kInput, kOutput, kInputPullUp, kInputPullDown };

// The function type required by `GpioConfigureInterrupt()`.
using GpioCallback = std::function<void()>;

// @cond Do not generate docs
void GpioInit();
// @endcond

// Sets the output value of a GPIO.
// @param gpio Pin to configure. Only pins in the `Gpio` enumeration can be
//  configured with this module. To use a GPIO that is not covered by this
//  module, use the functions in
//  `third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h`
// @param enable Whether to set the pin to high or low.
void GpioSet(Gpio gpio, bool enable);

// Gets the input value of a GPIO.
// @param gpio Pin to read.
// @returns Boolean representing high or low state of the pin.
bool GpioGet(Gpio gpio);

// Sets the mode of a GPIO.
// @param gpio Pin to configure.
// @param mode Mode to configure the gpio as.
void GpioSetMode(Gpio gpio, GpioMode mode);

// Sets the interrupt mode and callback for a GPIO.
//
// @param gpio Pin to configure.
// @param mode The style of interrupt to sense.
// @param cb Callback function that will be invoked when the interrupt is
// raised. This is called from interrupt context, so it should not do much
// work.
void GpioConfigureInterrupt(Gpio gpio, GpioInterruptMode mode, GpioCallback cb);

// Sets the interrupt mode, callback, and debounce interval for a GPIO.
//
// **Example** (from `examples/button_led/`):
//
// \snippet button_led/button_led.cc gpio-callback
//
// @param gpio Pin to configure.
// @param mode The style of interrupt to sense.
// @param cb Callback function that will be invoked when the interrupt is
// raised. This is called from interrupt context, so it should not do much
// work.
// @param debounce_interval_us Minimum interval in microseconds between repeated
// invocations of `cb`. Useful for cases where the GPIO line could toggle back
// and forth more frequently than expected, such as a mechanical button.
void GpioConfigureInterrupt(Gpio gpio, GpioInterruptMode mode, GpioCallback cb,
                            uint64_t debounce_interval_us);

}  // namespace coralmicro

#endif  // LIBS_BASE_GPIO_H_
