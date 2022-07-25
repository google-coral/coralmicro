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

#include <functional>

#include "libs/base/check.h"
#include "libs/base/console_m7.h"
#include "libs/base/gpio.h"
#include "libs/base/main_freertos_m7.h"
#include "libs/base/random.h"
#include "libs/base/reset.h"
#include "libs/base/tasks.h"
#include "libs/base/tempsense.h"
#include "libs/base/timer.h"
#include "libs/msc_ums/msc_ums.h"
#include "libs/nxp/rt1176-sdk/board_hardware.h"
#include "libs/pmic/pmic.h"
#include "libs/usb/usb_device_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_sema4.h"

namespace {
lpi2c_rtos_handle_t g_i2c5_handle;
coralmicro::MscUms g_msc_ums;

void InitializeUMS() {
  using namespace std::placeholders;
  g_msc_ums.Init(
      coralmicro::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
      coralmicro::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
      coralmicro::UsbDeviceTask::GetSingleton()->next_interface_value());
  coralmicro::UsbDeviceTask::GetSingleton()->AddDevice(
      g_msc_ums.config_data(),
      std::bind(&coralmicro::MscUms::SetClassHandle, &g_msc_ums, _1),
      std::bind(&coralmicro::MscUms::HandleEvent, &g_msc_ums, _1, _2),
      g_msc_ums.descriptor_data(), g_msc_ums.descriptor_data_size());
}
}  // namespace

extern "C" lpi2c_rtos_handle_t* I2C5Handle() { return &g_i2c5_handle; }

extern "C" void app_main(void* param);
extern "C" int main(int argc, char** argv) __attribute__((weak));

extern "C" int main(int argc, char** argv) {
  return real_main(argc, argv, true, true);
}

extern "C" int real_main(int argc, char** argv, bool init_console_tx,
                         bool init_console_rx) {
  BOARD_InitHardware(true);
  SEMA4_Init(SEMA4);
  coralmicro::ResetStoreStats();
  coralmicro::TimerInit();
  coralmicro::GpioInit();
  coralmicro::RandomInit();
  coralmicro::ConsoleM7::GetSingleton()->Init(init_console_tx, init_console_rx);
  InitializeUMS();
  coralmicro::UsbDeviceTask::GetSingleton()->Init();
  coralmicro::TempSensorInit();

  // Initialize I2C5 state
  NVIC_SetPriority(LPI2C5_IRQn, 3);
  lpi2c_master_config_t config;
  LPI2C_MasterGetDefaultConfig(&config);
  LPI2C_RTOS_Init(&g_i2c5_handle, reinterpret_cast<LPI2C_Type*>(LPI2C5_BASE),
                  &config, CLOCK_GetFreq(kCLOCK_OscRc48MDiv2));

  coralmicro::PmicTask::GetSingleton()->Init(&g_i2c5_handle);

  CHECK(xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE * 30,
                    nullptr, coralmicro::kAppTaskPriority, nullptr) == pdPASS);

  vTaskStartScheduler();
  return 0;
}
