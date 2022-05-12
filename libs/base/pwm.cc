#include "libs/base/pwm.h"

#include <cstdio>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pwm.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_xbara.h"

namespace coral::micro {
namespace pwm {

static pwm_module_control_t PwmModuleToControl(pwm_submodule_t module) {
    switch (module) {
        case kPWM_Module_0:
            return kPWM_Control_Module_0;
        case kPWM_Module_1:
            return kPWM_Control_Module_1;
        case kPWM_Module_2:
            return kPWM_Control_Module_2;
        case kPWM_Module_3:
            return kPWM_Control_Module_3;
        default:
            assert(false);
    }
}

void Init(const PwmModuleConfig& config) {
    static bool xbar_inited = false;
    if (!xbar_inited) {
        XBARA_Init(XBARA1);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh,
                                   kXBARA1_OutputFlexpwm1Fault0);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh,
                                   kXBARA1_OutputFlexpwm1Fault1);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh,
                                   kXBARA1_OutputFlexpwm1234Fault2);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh,
                                   kXBARA1_OutputFlexpwm1234Fault3);
        xbar_inited = true;
    }

    pwm_config_t pwm_config;
    PWM_GetDefaultConfig(&pwm_config);
    pwm_config.prescale = kPWM_Prescale_Divide_4;
    if (PWM_Init(config.base, config.module, &pwm_config) == kStatus_Fail) {
        printf("PWM_Init failed\r\n");
        return;
    }

    uint32_t source_clock_hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Bus);
    pwm_signal_param_t signal_param[2];
    int signal = 0;
    if (config.A.enabled) {
        signal_param[signal].pwmChannel = kPWM_PwmA;
        signal_param[signal].level = kPWM_HighTrue;
        signal_param[signal].dutyCyclePercent = config.A.duty_cycle;
        signal_param[signal].faultState = kPWM_PwmFaultState0;
        ++signal;
    }

    if (config.B.enabled) {
        signal_param[signal].pwmChannel = kPWM_PwmB;
        signal_param[signal].level = kPWM_HighTrue;
        signal_param[signal].dutyCyclePercent = config.B.duty_cycle;
        signal_param[signal].faultState = kPWM_PwmFaultState0;
        ++signal;
    }

    assert(signal > 0);
    PWM_SetupPwm(config.base, config.module, &signal_param[0], signal,
                 kPWM_SignedCenterAligned, 1000, source_clock_hz);

    PWM_SetPwmLdok(config.base, PwmModuleToControl(config.module), true);
}

void Enable(const PwmModuleConfig& config, bool enable) {
    if (enable) {
        PWM_StartTimer(config.base, PwmModuleToControl(config.module));
    } else {
        PWM_StopTimer(config.base, PwmModuleToControl(config.module));
    }
}

}  // namespace pwm
}  // namespace coral::micro