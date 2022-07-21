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

#include <cstring>

#include "libs/base/check.h"
#include "libs/base/console_m4.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/ipc_m4.h"
#include "libs/base/tasks.h"
#include "libs/base/timer.h"
#include "libs/camera/camera.h"
#include "libs/nxp/rt1176-sdk/board_hardware.h"
#include "libs/pmic/pmic.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

extern "C" void app_main(void* param);

namespace {
lpi2c_rtos_handle_t g_i2c5_handle;

void pre_app_main(void* param) {
  coralmicro::IpcM4::GetSingleton()->Init();
  coralmicro::ConsoleM4Init();
  app_main(param);
}
}  // namespace

extern "C" lpi2c_rtos_handle_t* I2C5Handle() { return &g_i2c5_handle; }

extern "C" int main(int argc, char** argv) __attribute__((weak));
extern "C" int main(int argc, char** argv) {
  BOARD_InitHardware(true);

  CHECK(coralmicro::LfsInit());
  coralmicro::GpioInit();
  coralmicro::TimerInit();

  // Initialize I2C5 state
  NVIC_SetPriority(LPI2C5_IRQn, 3);
  lpi2c_master_config_t config;
  LPI2C_MasterGetDefaultConfig(&config);
  LPI2C_RTOS_Init(&g_i2c5_handle, reinterpret_cast<LPI2C_Type*>(LPI2C5_BASE),
                  &config, CLOCK_GetFreq(kCLOCK_OscRc48MDiv2));

  coralmicro::PmicTask::GetSingleton()->Init(&g_i2c5_handle);

  constexpr size_t stack_size = configMINIMAL_STACK_SIZE * 10;
  static StaticTask_t xTaskBuffer;
  static StackType_t xStack[stack_size];
  CHECK(xTaskCreateStatic(pre_app_main, "app_main", stack_size, nullptr,
                          coralmicro::kAppTaskPriority, xStack, &xTaskBuffer));

  vTaskStartScheduler();
  return 0;
}
