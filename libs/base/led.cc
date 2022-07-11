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

namespace coralmicro {

bool LedSet(Led led, bool enable) {
    return LedSet(led, enable, enable ? kLedFullyOn : kLedFullyOff);
}

bool LedSet(Led led, bool enable, unsigned int brightness) {
    bool ret = true;
    switch (led) {
        case Led::kStatus:
            GpioSet(Gpio::kStatusLed, enable);
            break;
        case Led::kUser:
            GpioSet(Gpio::kUserLed, enable);
            break;
        case Led::kTpu:
#if __CORTEX_M == 7
            if (!GpioGet(Gpio::kEdgeTpuPmic)) {
                printf("TPU LED requires TPU power to be enabled.\r\n");
                ret = false;
                break;
            }
            if (brightness > kLedFullyOn) {
                brightness = kLedFullyOff;
            }
            PwmModuleConfig config;
            config.base = PWM1;
            config.module = kPWM_Module_1;
            if (enable) {
                config.A.enabled = true;
                config.A.duty_cycle = brightness;
                config.B.enabled = false;
                PwmInit(config);
                PwmEnable(config, true);
            } else {
                PwmEnable(config, false);
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

}  // namespace coralmicro
