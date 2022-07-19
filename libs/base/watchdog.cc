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

#include "libs/base/watchdog.h"

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/timers.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_wdog.h"

namespace coralmicro {
namespace {
TimerHandle_t wdog_timer;
}  // namespace

void WatchdogStart(const WatchdogConfig& config) {
  wdog_config_t wdog_config;
  // wdogConfig->enableWdog = true;
  // wdogConfig->workMode.enableWait = true;
  // wdogConfig->workMode.enableStop = false;
  // wdogConfig->workMode.enableDebug = false;
  // wdogConfig->enableInterrupt = false;
  // wdogConfig->enablePowerdown = false;
  // wdogConfig->resetExtension = flase;
  // wdogConfig->timeoutValue = 0xFFU;
  // wdogConfig->interruptTimeValue = 0x04u;
  WDOG_GetDefaultConfig(&wdog_config);
  wdog_config.timeoutValue =
      (config.timeout_s * 2) - 1;  // Timeout time  = (register_val + 1)/2.
  if (config.enable_irq) {
    wdog_config.enableInterrupt = true;
    wdog_config.interruptTimeValue = (config.irq_s_before_timeout * 2) - 1;
  }
  WDOG_Init(WDOG1, &wdog_config);
  EnableIRQ(WDOG1_IRQn);
  NVIC_SetPriority(WDOG1_IRQn, 2);

  wdog_timer = xTimerCreate(
      "wdog_timer", pdMS_TO_TICKS(config.pet_rate_s * 1000), pdTRUE,
      /*pvTimerID=*/nullptr, +[](TimerHandle_t xTimer) {
        WDOG_Refresh(WDOG1);
        // The timer automatically restarts.
      });
  xTimerStart(wdog_timer, 0);
}

void WatchdogStop() {
  DisableIRQ(WDOG1_IRQn);
  WDOG_Deinit(WDOG1);
  xTimerDelete(wdog_timer, /*xTicksToWait=*/0);
}
}  // namespace coralmicro
