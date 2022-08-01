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

#ifndef LIBS_BASE_LED_H_
#define LIBS_BASE_LED_H_

namespace coralmicro {

// Available board LEDs
enum class Led {
  // Orange status LED.
  kStatus,
  // Green user-programmable LED.
  kUser,
  // White Edge TPU LED. The Edge TPU must be powered to use this.
  kTpu,
};

// Fully-on brightness for `Set(led, enable, brightness)`
inline constexpr int kLedFullyOn = 100;

// Fully-off brightness for `Set(led, enable, brightness)`
inline constexpr int kLedFullyOff = 0;

// Turns an LED on or off.
//
// @param led The LED to enable/disable.
// @param enable True turns the LED on, false turns it off.
// @returns True upon success, false otherwise.
bool LedSet(Led led, bool enable);

// Turns an LED on or off with brightness setting.
//
// @param led The LED to enable/disable.
// @param brightness The LED brightness, from 0 to 100.
// @returns True upon success, false otherwise.
bool LedSetBrightness(Led led, int brightness);

}  // namespace coralmicro

#endif  // LIBS_BASE_LED_H_
