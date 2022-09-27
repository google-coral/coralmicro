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

#include <algorithm>

#include "libs/base/gpio.h"
#include "libs/base/pwm.h"

namespace coralmicro {

bool LedSet(Led led, bool enable) {
  return LedSetBrightness(led, enable ? kLedFullyOn : kLedFullyOff);
}

bool LedSetBrightness(Led led, int brightness) {
  brightness = std::clamp(brightness, kLedFullyOff, kLedFullyOn);
  bool enable = brightness > kLedFullyOff;

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
      PwmInit();
      if (!GpioGet(Gpio::kEdgeTpuPmic)) {
        printf("TPU LED requires TPU power to be enabled.\r\n");
        ret = false;
        break;
      }
      coralmicro::PwmPinConfig pin_a_config;
      pin_a_config.duty_cycle = brightness;
      pin_a_config.frequency = 1000;
      pin_a_config.pin_setting =
          coralmicro::PwmPinSettingFor(coralmicro::PwmPin::k10);
      pin_a_config.pin_setting.sub_module = kPWM_Module_1;
      if (enable) {
        PwmEnable({pin_a_config});
      } else {
        PwmDisable({pin_a_config});
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
