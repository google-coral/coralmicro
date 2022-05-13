#include "libs/base/analog.h"

#include <cstdio>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dac12.h"

namespace coral::micro {
namespace analog {
namespace {
constexpr int kLPADC1ChannelCount = 6;
constexpr int kLPADC2ChannelCount = 7;

ADC_Type* DeviceToADC(Device d) {
    switch (d) {
        case Device::ADC1:
            return LPADC1;
        case Device::ADC2:
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
    if (device == Device::ADC1 || device == Device::ADC2) {
        lpadc_config_t adc_config;
        LPADC_GetDefaultConfig(&adc_config);
        LPADC_Init(DeviceToADC(device), &adc_config);
    }

    if (device == Device::DAC1) {
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
            case Side::A:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelDiffBothSideAB;
            case Side::B:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelDiffBothSideBA;
        }
    } else {
        switch (primary_side) {
            case Side::A:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelSingleEndSideA;
            case Side::B:
                config.conv_config.sampleChannelMode =
                    kLPADC_SampleChannelSingleEndSideB;
        }
    }

    if (device == Device::ADC1) {
        assert(channel < kLPADC1ChannelCount);
    }
    if (device == Device::ADC2) {
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

void WriteDAC(uint16_t counts) {
    // 12-bit DAC, values range from 0 - 4095.
    assert(counts <= 4095);
    DAC12_SetData(DAC, counts);
}

}  // namespace analog
}  // namespace coral::micro
