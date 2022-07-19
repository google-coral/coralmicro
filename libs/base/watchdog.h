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

struct WatchdogConfig {
  int timeout_s;
  int pet_rate_s;
  bool enable_irq;
  int irq_s_before_timeout;
};

// If enabling the interrupt, be sure to extern WDOG1_IRQHandler. Otherwise
// you will call DefaultISR.
void WatchdogStart(const WatchdogConfig& config);
void WatchdogStop();

}  // namespace coralmicro

#endif  // LIBS_BASE_WATCHDOG_H_
