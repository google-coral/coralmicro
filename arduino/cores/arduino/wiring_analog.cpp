#include "Arduino.h"
#include "wiring_private.h"

#include "libs/base/analog.h"
#include "libs/base/pwm.h"

#include <algorithm>
#include <cassert>

using valiant::analog::ADCConfig;
using valiant::analog::Device;
using valiant::analog::Side;
using valiant::pwm::PwmModuleConfig;
using valiant::pwm::PwmPinConfig;

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
    valiant::analog::Init(Device::ADC1);
    valiant::analog::CreateConfig(
        Config_A0,
        Device::ADC1, 0,
        Side::B, false
    );
    valiant::analog::CreateConfig(
        Config_A1,
        Device::ADC1, 0,
        Side::A, false
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

int analogRead(pin_size_t pinNumber) {
    int adc_shift = kAdcFullResolutionBits - adc_resolution_bits;
    const ADCConfig& config = pinToADCConfig(pinNumber);
    return (valiant::analog::ReadADC(config) >> adc_shift);
}

void analogReference(uint8_t mode) {}

void analogWrite(pin_size_t pinNumber, int value) {
    assert(value <= 255);
    PwmPinConfig *pin_config;
    if (pinNumber == A3) {
        pin_config = &pwm_config.A;
    } else if (pinNumber == A4) {
        pin_config = &pwm_config.B;
    } else {
        assert(false);
    }
    pin_config->enabled = true;
    pin_config->duty_cycle = map(value, 0, 255, 0, 100);
    valiant::pwm::Init(pwm_config);
    valiant::pwm::Enable(pwm_config, true);
}

void analogReadResolution(int bits) {
    adc_resolution_bits = std::max(1, std::min(bits, kAdcFullResolutionBits));
}

void analogWriteResolution(int bits) {
    dac_resolution_bits = std::max(1, std::min(bits, kDacFullResolutionBits));
}