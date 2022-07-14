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

#include <algorithm>
#include <cassert>

#include "Arduino.h"
#include "libs/base/analog.h"
#include "libs/base/led.h"
#include "libs/base/pwm.h"
#include "wiring_private.h"

using coralmicro::AdcConfig;
using coralmicro::AdcDevice;
using coralmicro::AdcSide;
using coralmicro::PwmPinConfig;

static constexpr int kAdcFullResolutionBits = 12;
// 12-bits for the real DAC (A2).
static constexpr int kDacFullResolutionBits = 12;
static int adc_resolution_bits = 10;
static int dac_resolution_bits = 8;
static AdcConfig Config_A0;
static AdcConfig Config_A1;

void wiringAnalogInit() {
    coralmicro::PwmInit();
    coralmicro::AdcInit(AdcDevice::kAdc1);
    coralmicro::DacInit();
    coralmicro::AdcCreateConfig(Config_A0, AdcDevice::kAdc1, 0, AdcSide::kB,
                                false);
    coralmicro::AdcCreateConfig(Config_A1, AdcDevice::kAdc1, 0, AdcSide::kA,
                                false);
}

const AdcConfig& pinToADCConfig(pin_size_t pinNumber) {
    switch (pinNumber) {
        case A0:
            return Config_A0;
        case A1:
            return Config_A1;
    }
    assert(false);
}

static void analogWriteDAC(pin_size_t pinNumber, int value) {
    assert(pinNumber == DAC0);
    int dac_shift = kDacFullResolutionBits - dac_resolution_bits;
    int shift_value = dac_resolution_bits;
    if (value) {
        coralmicro::DacEnable(true);
        coralmicro::DacWrite(value << dac_shift);
    } else {
        coralmicro::DacEnable(false);
    }
}

int analogRead(pin_size_t pinNumber) {
    int adc_shift = kAdcFullResolutionBits - adc_resolution_bits;
    const AdcConfig& config = pinToADCConfig(pinNumber);
    return (coralmicro::AdcRead(config) >> adc_shift);
}

void analogReference(uint8_t mode) {}

void analogWrite(pin_size_t pinNumber, int value) {
    assert(value <= 255);
    coralmicro::PwmPinSetting pwm_pin_setting;
    switch (pinNumber) {
        case A3:  // Pwm pin 10.
            pwm_pin_setting =
                coralmicro::PwmGetPinSetting(coralmicro::kPwmPin10).value();
            break;
        case A4:  // Pwm pin 9.
            pwm_pin_setting =
                coralmicro::PwmGetPinSetting(coralmicro::kPwmPin9).value();
            break;
        case DAC0:
            analogWriteDAC(pinNumber, value);
            return;
        case PIN_LED_TPU:
            coralmicro::LedSet(coralmicro::Led::kTpu, true,
                               map(value, 0, 255, coralmicro::kLedFullyOff,
                                   coralmicro::kLedFullyOn));
            return;
        default:
            assert(false);
    }
    coralmicro::PwmPinConfig pin_config{
        /*duty_cycle=*/map(value, 0, 255, 0, 100),
        /*pin_setting=*/pwm_pin_setting};
    coralmicro::PwmEnable({pin_config});
}

void analogReadResolution(int bits) {
    adc_resolution_bits = std::max(1, std::min(bits, kAdcFullResolutionBits));
}

void analogWriteResolution(int bits) {
    dac_resolution_bits = std::max(1, std::min(bits, kDacFullResolutionBits));
}
