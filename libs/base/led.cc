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

#include "libs/base/led.h"

#include "libs/base/gpio.h"
#include "libs/base/pwm.h"

namespace coral::micro {
namespace led {

bool Set(LED led, bool enable) {
    return Set(led, enable, enable ? kFullyOn : kFullyOff);
}

bool Set(LED led, bool enable, unsigned int brightness) {
    bool ret = true;
    switch (led) {
        case LED::kStatus:
            gpio::SetGpio(gpio::Gpio::kStatusLED, enable);
            break;
        case LED::kUser:
            gpio::SetGpio(gpio::Gpio::kUserLED, enable);
            break;
        case LED::kTpu:
#if __CORTEX_M == 7
            if (!gpio::GetGpio(gpio::Gpio::kEdgeTpuPmic)) {
                printf("TPU LED requires TPU power to be enabled.\r\n");
                ret = false;
                break;
            }
            if (brightness > kFullyOn) {
                brightness = kFullyOff;
            }
            pwm::PwmModuleConfig config;
            config.base = PWM1;
            config.module = kPWM_Module_1;
            if (enable) {
                config.A.enabled = true;
                config.A.duty_cycle = brightness;
                config.B.enabled = false;
                pwm::Init(config);
                pwm::Enable(config, true);
            } else {
                pwm::Enable(config, false);
            }
#else
            ret = false;
#endif
            break;
        default:
            ret = false;
    }
    return ret;
}

}  // namespace led
}  // namespace coral::micro
