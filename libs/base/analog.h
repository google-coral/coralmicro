#ifndef _LIBS_BASE_ANALOG_H_
#define _LIBS_BASE_ANALOG_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpadc.h"
#include <cstdint>

namespace coral::micro {
namespace analog {

enum class Device {
    ADC1,
    ADC2,
    DAC1,
};

enum class Side {
    A,
    B,
};

struct ADCConfig {
    ADC_Type *device;
    lpadc_conv_command_config_t conv_config;
    lpadc_conv_trigger_config_t trigger_config;
};

void Init(Device device);
void CreateConfig(ADCConfig& config, Device device, int channel, Side primary_side, bool differential);
uint16_t ReadADC(const ADCConfig& config);

void EnableDAC(bool enable);
void WriteDAC(uint16_t counts);

}  // namespace analog
}  // namespace coral::micro

#endif  // _LIBS_BASE_ANALOG_H_
