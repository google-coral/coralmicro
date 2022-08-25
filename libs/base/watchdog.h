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

#ifndef LIBS_BASE_WATCHDOG_H_
#define LIBS_BASE_WATCHDOG_H_

namespace coralmicro {

// Represents a watchdog config, this config needs to be passed into
// `WatchdogStart()`.
struct WatchdogConfig {
  // Number of seconds before the watchdog resets the CPU (unless pet).
  int timeout_s;
  // The rate to refresh the watchdog timer in seconds.
  int pet_rate_s;
  // Set to true to enable the watchdog interrupt. If watchdog interrupt is
  // enabled, be sure to implement and extern WDOG1_IRQHandler. Otherwise, you
  // will call DefaultISR.
  bool enable_irq;
  // When enable_irq is set, time remaining before watchdog expiration that will
  // trigger an interrupt (e.g. timeout_s = 10, irq_s_before_timeout=2 means the
  // interrupt will fire at 8 seconds).
  int irq_s_before_timeout;
};

// Enables watchdog and starts SW timer to refresh at a given rate.
//
// @param config The watchdog config to enable watchdog.
void WatchdogStart(const WatchdogConfig& config);

// Stops SW refresh timer, disables watchdog.
void WatchdogStop();

}  // namespace coralmicro

#endif  // LIBS_BASE_WATCHDOG_H_
