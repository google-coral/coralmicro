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

#include "Arduino.h"
#include "wiring_private.h"

#include "libs/base/analog.h"
#include "libs/base/led.h"
#include "libs/base/pwm.h"

#include <algorithm>
#include <cassert>

using coralmicro::analog::ADCConfig;
using coralmicro::analog::Device;
using coralmicro::analog::Side;
using coralmicro::PwmModuleConfig;
using coralmicro::PwmPinConfig;

static constexpr int kAdcFullResolutionBits = 12;
// 12-bits for the real DAC (A2).
static constexpr int kDacFullResolutionBits = 12;
static int adc_resolution_bits = 10;
static int dac_resolution_bits = 8;
static ADCConfig Config_A0;
static ADCConfig Config_A1;
static PwmModuleConfig pwm_config;


void wiringAnalogInit() {
    pwm_config.base = PWM1;
    pwm_config.module = kPWM_Module_0;
    pwm_config.A.enabled = false;
    pwm_config.B.enabled = false;
    coralmicro::analog::Init(Device::kAdc1);
    coralmicro::analog::Init(Device::kDac1);
    coralmicro::analog::CreateConfig(
        Config_A0,
        Device::kAdc1, 0,
        Side::kB, false
    );
    coralmicro::analog::CreateConfig(
        Config_A1,
        Device::kAdc1, 0,
        Side::kA, false
    );
}

const ADCConfig& pinToADCConfig(pin_size_t pinNumber) {
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
        coralmicro::analog::EnableDAC(true);
        coralmicro::analog::WriteDAC(value << dac_shift);
    } else {
        coralmicro::analog::EnableDAC(false);
    }
}

int analogRead(pin_size_t pinNumber) {
    int adc_shift = kAdcFullResolutionBits - adc_resolution_bits;
    const ADCConfig& config = pinToADCConfig(pinNumber);
    return (coralmicro::analog::ReadADC(config) >> adc_shift);
}

void analogReference(uint8_t mode) {}

void analogWrite(pin_size_t pinNumber, int value) {
    assert(value <= 255);
    PwmPinConfig *pin_config;
    if (pinNumber == A3) {
        pin_config = &pwm_config.A;
    } else if (pinNumber == A4) {
        pin_config = &pwm_config.B;
    } else if (pinNumber == DAC0) {
        analogWriteDAC(pinNumber, value);
        return;
    } else if (pinNumber == PIN_LED_TPU) {
        coralmicro::LedSet(coralmicro::Led::kTpu, true,
                               map(value,
                                   0, 255,
                                   coralmicro::kLedFullyOff,
                                   coralmicro::kLedFullyOn));
        return;
    } else {
        assert(false);
    }
    pin_config->enabled = true;
    pin_config->duty_cycle = map(value, 0, 255, 0, 100);
    coralmicro::PwmInit(pwm_config);
    coralmicro::PwmEnable(pwm_config, true);
}

void analogReadResolution(int bits) {
    adc_resolution_bits = std::max(1, std::min(bits, kAdcFullResolutionBits));
}

void analogWriteResolution(int bits) {
    dac_resolution_bits = std::max(1, std::min(bits, kDacFullResolutionBits));
}
