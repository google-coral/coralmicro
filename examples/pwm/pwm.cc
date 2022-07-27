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

#include "libs/base/pwm.h"

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Runs simple PWM cycles with pins PWM_A (pin 10 on the left-side header)
// and PWM_B (pin 9 on the left-side header).
// Note: These pins output a max of 1.8V

// [start-sphinx-snippet:pwm]
extern "C" [[noreturn]] void app_main(void* param) {
  (void)param;

  coralmicro::PwmInit();
  coralmicro::PwmPinConfig pin_a_config{
      /*duty_cycle=*/20,
      /*frequency=*/1000,
      /*pin_setting=*/
      coralmicro::PwmGetPinSetting(coralmicro::kPwmPin10).value()};
  coralmicro::PwmPinConfig pin_b_config{
      /*duty_cycle=*/80,
      /*frequency=*/1000,
      /*pin_setting=*/
      coralmicro::PwmGetPinSetting(coralmicro::kPwmPin9).value()};
  std::vector<coralmicro::PwmPinConfig> configs = {pin_a_config, pin_b_config};
  while (true) {
    coralmicro::PwmEnable(configs);
    vTaskDelay(pdMS_TO_TICKS(1000));
    coralmicro::PwmDisable(configs);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
// [end-sphinx-snippet:pwm]
