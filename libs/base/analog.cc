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

#include "libs/base/analog.h"

#include <cstdio>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dac12.h"

namespace coralmicro {
namespace analog {
namespace {

ADC_Type* DeviceToADC(Device d) {
    switch (d) {
        case Device::kAdc1:
            return LPADC1;
        case Device::kAdc2:
            return LPADC2;
        default:
            return nullptr;
    }
    assert(false);
}

void GetDefaultConfig(ADCConfig& config) {
    LPADC_GetDefaultConvCommandConfig(&config.conv_config);
    LPADC_GetDefaultConvTriggerConfig(&config.trigger_config);
}
} // namespace

void Init(Device device) {
    if (device == Device::kAdc1 || device == Device::kAdc2) {
        lpadc_config_t adc_config;
        LPADC_GetDefaultConfig(&adc_config);
        LPADC_Init(DeviceToADC(device), &adc_config);
    }

    if (device == Device::kDac1) {
        dac12_config_t dac_config;
        DAC12_GetDefaultConfig(&dac_config);
        dac_config.referenceVoltageSource = kDAC12_ReferenceVoltageSourceAlt2;
        DAC12_Init(DAC, &dac_config);
    }
}

void CreateConfig(ADCConfig& config, Device device, int channel,
                  Side primary_side, bool differential) {
    GetDefaultConfig(config);
    config.device = DeviceToADC(device);

    if (differential) {
        switch (primary_side) {
            case Side::kA:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelDiffBothSideAB;
            case Side::kB:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelDiffBothSideBA;
        }
    } else {
        switch (primary_side) {
            case Side::kA:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelSingleEndSideA;
            case Side::kB:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelSingleEndSideB;
        }
    }

    if (device == Device::kAdc1) {
        assert(channel < kLPADC1ChannelCount);
    }
    if (device == Device::kAdc2) {
        assert(channel < kLPADC2ChannelCount);
    }

    config.conv_config.channelNumber = channel;
    config.trigger_config.targetCommandId = 1;
    config.trigger_config.enableHardwareTrigger = false;
}

uint16_t ReadADC(const ADCConfig& config) {
    lpadc_conv_result_t result;

    LPADC_SetConvCommandConfig(config.device, 1, &config.conv_config);
    LPADC_SetConvTriggerConfig(config.device, 0, &config.trigger_config);
    LPADC_DoSoftwareTrigger(config.device, 1);
    while (!LPADC_GetConvResult(config.device, &result)) {
    }

    return (result.convValue >> 3) & 0xFFF;
}

void EnableDAC(bool enable) { DAC12_Enable(DAC, enable); }

void WriteDAC(uint16_t value) {
    // 12-bit DAC, values range from 0 - 4095.
    assert(value <= 4095);
    DAC12_SetData(DAC, value);
}

}  // namespace analog
}  // namespace coralmicro
