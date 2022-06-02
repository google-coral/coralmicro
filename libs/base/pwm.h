#ifndef LIBS_BASE_PWM_H_
#define LIBS_BASE_PWM_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pwm.h"

namespace coral::micro {
namespace pwm {

// Represents the configuration for a single PWM pin.
struct PwmPinConfig {
    // The desired state of the pin.
    bool enabled;
    // The duty cycle (from 0-100) of the PWM waveform.
    int duty_cycle;
};

// Represents the configuration of an entire PWM module.
struct PwmModuleConfig {
    // Pointer to the base register of the PWM module.
    PWM_Type* base;
    // Submodule to configure.
    pwm_submodule_t module;
    // Configuration for pin A.
    PwmPinConfig A;
    // Configuration for pin B.
    PwmPinConfig B;
};

// Initializes a PWM module.
// @param config The configuration structure for the module.
void Init(const PwmModuleConfig& config);

// Enables or disables a PWM module.
// @param config The configuration structure for the module.
// @param enable True to enable the module; false to disable it.
void Enable(const PwmModuleConfig& config, bool enable);

}  // namespace pwm
}  // namespace coral::micro

#endif  // LIBS_BASE_PWM_H_
