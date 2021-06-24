#include "Arduino.h"
#include "wiring_private.h"

#include "libs/base/analog.h"

#include <algorithm>
#include <cassert>

using valiant::analog::ADCConfig;
using valiant::analog::Device;
using valiant::analog::Side;

static constexpr int kAdcFullResolutionBits = 12;
// 12-bits for the real DAC (A2).
static constexpr int kDacFullResolutionBits = 12;
static int adc_resolution_bits = 10;
static int dac_resolution_bits = 8;
static ADCConfig Config_A0;
static ADCConfig Config_A1;

void wiringAnalogInit() {
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
    assert(false);
}

void analogReadResolution(int bits) {
    adc_resolution_bits = std::max(1, std::min(bits, kAdcFullResolutionBits));
}

void analogWriteResolution(int bits) {
    dac_resolution_bits = std::max(1, std::min(bits, kDacFullResolutionBits));
}