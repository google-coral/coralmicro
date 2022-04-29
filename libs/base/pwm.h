#ifndef _LIBS_BASE_PWM_H_
#define _LIBS_BASE_PWM_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pwm.h"

namespace coral::micro {
namespace pwm {

struct PwmPinConfig {
    bool enabled;
    int duty_cycle;
};

struct PwmModuleConfig {
    PWM_Type* base;
    pwm_submodule_t module;
    PwmPinConfig A;
    PwmPinConfig B;
};

void Init(const PwmModuleConfig& config);
void Enable(const PwmModuleConfig& config, bool enable);

}  // namespace pwm
}  // namespace coral::micro

#endif  // _LIBS_BASE_PWM_H_