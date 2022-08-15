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

#ifndef LIBS_BASE_ANALOG_H_
#define LIBS_BASE_ANALOG_H_

#include <cstdint>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpadc.h"

namespace coralmicro {

// Available channels on ADC1.
inline constexpr int kAdc1ChannelCount = 6;

// Choices for the primary side of an ADC.
enum class AdcSide {
  // ADC1_CH0A (GPIO_AD_06); left header, pin 3
  kA,
  // ADC1_CH0B (GPIO_AD_07); left header, pin 4
  kB,
};

// Represents the configuration of an ADC.
// Each ADC has a 12-bit resolution with 1.8V reference voltage.
struct AdcConfig {
  // Pointer to the base register of the ADC.
  ADC_Type* device;
  // Configuration for ADC conversion.
  lpadc_conv_command_config_t conv_config;
  // Configuration for ADC triggers.
  lpadc_conv_trigger_config_t trigger_config;
};

// Initializes ADC device.
void AdcInit();

// Populates an `ADCConfig` struct based on the given parameters.
// @param config Configuration struct to populate.
// @param channel The ADC channel to use (must be less than the max number of
// channels: `kAdc1ChannelCount`).
// @param primary_side In single ended mode, this is the pin that's connected.
//   In differential mode, this is the pin to use as the primary side.
// @param differential Whether or not to run the ADC in differential mode.
void AdcCreateConfig(AdcConfig& config, int channel, AdcSide primary_side,
                     bool differential);

// Reads voltage values from an ADC.
//
// @param config ADC configuration to use.
// @returns Digitized value of the voltage that the ADC is sensing.
//   The ADC has 12 bits of precision, so the maximum value returned is 4095.
uint16_t AdcRead(const AdcConfig& config);

// Initializes DAC device.
// @param device DAC to initialize.
void DacInit();

// Sets the state of the DAC. It is recommended that you set
// a voltage with `DacWrite` prior to enabling the DAC.
// @param enable True enables the DAC; false disables it.
void DacEnable(bool enable);

// Writes voltage values to the DAC.
//
// For example code, see `examples/analog/`.
// @param value The voltage value to output.
//   The DAC has 12-bit resolution, so the allowed values are 0 to 4095.
//   The maximum output voltage of the DAC is 1.8V.
void DacWrite(uint16_t value);

}  // namespace coralmicro

#endif  // LIBS_BASE_ANALOG_H_
