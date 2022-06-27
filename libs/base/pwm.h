// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
