#ifndef LIBS_BASE_ANALOG_H_
#define LIBS_BASE_ANALOG_H_

#include <cstdint>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpadc.h"

namespace coral::micro {
namespace analog {

// Available channels on ADC1.
constexpr int kLPADC1ChannelCount = 6;
// Available channels on ADC2.
constexpr int kLPADC2ChannelCount = 7;

// Enumeration of the ADCs and DACs that exist in the system.
enum class Device {
    ADC1,
    ADC2,
    DAC1,
};

// Enumeration of the choices for the primary side of an ADC.
enum class Side {
    A,
    B,
};

// Represents the configuration of an ADC.
// Each ADC has a 12-bit resolution with 1.8V reference voltage.
struct ADCConfig {
    // Pointer to the base register of the ADC.
    ADC_Type* device;
    // Configuration for ADC conversion.
    lpadc_conv_command_config_t conv_config;
    // Configuration for ADC triggers.
    lpadc_conv_trigger_config_t trigger_config;
};

// Initializes an analog device.
// @param device ADC or DAC to initialize.
void Init(Device device);

// Populates an `ADCConfig` struct based on the given parameters.
// @param config Configuration struct to populate.
// @param device Desired device to configure.
// @param channel The ADC channel to use (must be less than the max number of channels).
//   See `kLPADC1ChannelCount` and `kLPADC2ChannelCount` for the amount of channels.
// @param primary_side Primary side of the ADC, if using differential mode.
//   In single ended mode, the desired side.
// @param differential Whether or not to run the ADC in differential mode.
void CreateConfig(ADCConfig& config, Device device, int channel,
                  Side primary_side, bool differential);

// Reads voltage values from an ADC.
//
// For example code, see `examples/analog/`.
// @param config ADC configuration to use.
// @returns Digitized value of the voltage that the ADC is sensing.
//          The ADC has 12 bits of precision, so the maximum value returned is 4095.
uint16_t ReadADC(const ADCConfig& config);

// Sets the state of the DAC.
// @param enable True enables the DAC; false disables it.
void EnableDAC(bool enable);

// Writes voltage values to the DAC. You must first call `EnableDAC()`.
//
// For example code, see `examples/analog/`.
// @param value The voltage value to output.
//   The DAC has 12-bit resolution, so the allowed values are 0 to 4095.
//   The maximum output voltage of the DAC is 1.8V.
void WriteDAC(uint16_t value);

}  // namespace analog
}  // namespace coral::micro

#endif  // LIBS_BASE_ANALOG_H_
