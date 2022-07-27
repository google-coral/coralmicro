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

#include "libs/base/pwm.h"

#include <cstdio>

#include "libs/base/check.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pwm.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_xbara.h"

namespace coralmicro {
namespace {
pwm_module_control_t PwmModuleToControl(pwm_submodule_t module) {
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

}  // namespace

PwmPinSetting PwmPinSettingFor(PwmPin pin) {
  PwmPinSetting setting{};
  setting.base = PWM1;
  setting.sub_module = kPWM_Module_0;
  switch (pin) {
    case PwmPin::k10:
      setting.pwm_channel = kPWM_PwmA;
      break;
    case PwmPin::k9:
      setting.pwm_channel = kPWM_PwmB;
      break;
    default:
      CHECK(!"Invalid pin");
  }
  return setting;
}

void PwmInit() {
  static bool muxed{false};
  if (!muxed) {
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_00_FLEXPWM1_PWM0_A, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_01_FLEXPWM1_PWM0_B, 0U);
  }
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
}

void PwmEnable(const std::vector<PwmPinConfig>& pin_configs) {
  if (pin_configs.empty()) return;

  // Each pwm submodule only has 2 pins to output PWM.
  CHECK(pin_configs.size() <= 2);
  if (pin_configs.size() == 2) {
    CHECK(pin_configs[0].pin_setting.base == pin_configs[1].pin_setting.base);
    CHECK(pin_configs[0].pin_setting.sub_module ==
          pin_configs[1].pin_setting.sub_module);
  }

  const auto& base_addr = pin_configs[0].pin_setting.base;
  const auto& sub_module = pin_configs[0].pin_setting.sub_module;
  pwm_config_t pwm_config;
  PWM_GetDefaultConfig(&pwm_config);
  pwm_config.prescale = kPWM_Prescale_Divide_4;
  if (PWM_Init(base_addr, sub_module, &pwm_config) == kStatus_Fail) {
    printf("PWM_Init failed\r\n");
    return;
  }
  pwm_signal_param_t signal_param[pin_configs.size()];
  auto source_clock_hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Bus);
  for (size_t i = 0; i < pin_configs.size(); ++i) {
    signal_param[i].pwmChannel = pin_configs[i].pin_setting.pwm_channel;
    signal_param[i].level = kPWM_HighTrue;
    signal_param[i].dutyCyclePercent = pin_configs[i].duty_cycle;
    signal_param[i].faultState = kPWM_PwmFaultState0;
    PWM_SetupPwm(base_addr, sub_module, &signal_param[i], 1,
                 kPWM_SignedCenterAligned, pin_configs[i].frequency,
                 source_clock_hz);
  }
  PWM_SetPwmLdok(base_addr, PwmModuleToControl(sub_module), true);
  PWM_StartTimer(base_addr, PwmModuleToControl(sub_module));
}

void PwmDisable(const std::vector<PwmPinConfig>& pin_configs) {
  if (pin_configs.empty()) return;
  for (const auto& pin_config : pin_configs) {
    PWM_StopTimer(pin_config.pin_setting.base,
                  PwmModuleToControl(pin_config.pin_setting.sub_module));
  }
}
}  // namespace coralmicro
