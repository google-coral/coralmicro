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

#include "libs/base/i2c.h"

#include "./third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "libs/base/check.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"

namespace coralmicro {

namespace {
// Interrupt context!!!
void I2cTargetCallback(LPI2C_Type* base, lpi2c_target_transfer_t* transfer,
                       void* userData) {
  auto config = static_cast<I2cConfig*>(userData);
  CHECK(config->base == base);
  switch (transfer->event) {
    case kLPI2C_SlaveAddressMatchEvent:
      break;
    case kLPI2C_SlaveTransmitEvent:
    case kLPI2C_SlaveReceiveEvent:
      config->target_callback(config, transfer);
      break;
    default:
      break;
  }
}

void I2cInit(I2cConfig& config, bool controller) {
  IOMUXC_SetPinMux(config.sda[0], config.sda[1], config.sda[2], config.sda[3],
                   config.sda[4], 1U);
  IOMUXC_SetPinMux(config.scl[0], config.scl[1], config.scl[2], config.scl[3],
                   config.scl[4], 1U);
  NVIC_SetPriority(static_cast<IRQn_Type>(config.interrupt), 3);
  config.controller = controller;
  if (controller) {
    LPI2C_MasterGetDefaultConfig(&config.controller_config);
  } else {
    LPI2C_SlaveGetDefaultConfig(&config.target_config);
  }
}
}  // namespace

I2cConfig I2cGetDefaultConfig(I2c bus) {
  I2cConfig config{};
  switch (bus) {
    case I2c::kI2c1:
      config.base = LPI2C1;
      config.sda = {IOMUXC_GPIO_AD_33_LPI2C1_SDA};
      config.scl = {IOMUXC_GPIO_AD_32_LPI2C1_SCL};
      config.interrupt = LPI2C1_IRQn;
      config.clk_freq = CLOCK_GetFreqFromObs(CCM_OBS_LPI2C1_CLK_ROOT);
      break;
    case I2c::kI2c6:
      config.base = LPI2C6;
      config.sda = {IOMUXC_GPIO_LPSR_06_LPI2C6_SDA};
      config.scl = {IOMUXC_GPIO_LPSR_07_LPI2C6_SCL};
      config.interrupt = LPI2C6_IRQn;
      config.clk_freq = CLOCK_GetFreqFromObs(CCM_OBS_LPI2C6_CLK_ROOT);
      break;
    default:
      CHECK(false);
  }
  return config;
}

bool I2cInitController(I2cConfig& config) {
  I2cInit(config, true);
  return (LPI2C_RTOS_Init(&config.controller_handle, config.base,
                          &config.controller_config,
                          config.clk_freq) == kStatus_Success);
}

bool I2cInitTarget(I2cConfig& config, uint8_t address,
                   TargetCallback callback) {
  I2cInit(config, false);
  config.target_config.address0 = address;
  config.target_callback = callback;
  LPI2C_SlaveInit(config.base, &config.target_config, config.clk_freq);
  LPI2C_SlaveTransferCreateHandle(config.base, &config.target_handle,
                                  I2cTargetCallback, &config);
  status_t status = LPI2C_SlaveTransferNonBlocking(
      config.base, &config.target_handle,
      kLPI2C_SlaveCompletionEvent | kLPI2C_SlaveAddressMatchEvent);
  return (status == kStatus_Success);
}

bool I2cControllerRead(I2cConfig& config, uint8_t address, uint8_t* buffer,
                       size_t count) {
  CHECK(config.controller);
  lpi2c_master_transfer_t transfer;
  transfer.flags = kLPI2C_TransferDefaultFlag;
  transfer.slaveAddress = address;
  transfer.direction = kLPI2C_Read;
  transfer.subaddress = 0;
  transfer.subaddressSize = 0;
  transfer.data = buffer;
  transfer.dataSize = count;

  status_t status = LPI2C_RTOS_Transfer(&config.controller_handle, &transfer);
  return (status == kStatus_Success);
}

bool I2cControllerWrite(I2cConfig& config, uint8_t address, uint8_t* buffer,
                        size_t count) {
  CHECK(config.controller);
  lpi2c_master_transfer_t transfer;
  transfer.flags = kLPI2C_TransferDefaultFlag;
  transfer.slaveAddress = address;
  transfer.direction = kLPI2C_Write;
  transfer.subaddress = 0;
  transfer.subaddressSize = 0;
  transfer.data = buffer;
  transfer.dataSize = count;

  status_t status = LPI2C_RTOS_Transfer(&config.controller_handle, &transfer);
  return (status == kStatus_Success);
}

}  // namespace coralmicro
