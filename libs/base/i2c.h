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

#ifndef LIBS_BASE_I2C_H_
#define LIBS_BASE_I2C_H_

#include <array>
#include <cstdint>
#include <functional>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coralmicro {

typedef lpi2c_slave_config_t lpi2c_target_config_t;
typedef lpi2c_slave_transfer_t lpi2c_target_transfer_t;
typedef lpi2c_slave_handle_t lpi2c_target_handle_t;

// Configuration for an I2C interface.
struct I2cConfig {
  // Base pointer to the I2C peripheral.
  LPI2C_Type* base;
  // Interrupt number for the peripheral.
  int interrupt;
  // IOMUXC configuration for the SDA pin.
  std::array<uint32_t, 5> sda;
  // IOMUXC configuration for the SCL pin.
  std::array<uint32_t, 5> scl;
  // True if the interface will be the controller; false if target.
  bool controller;
  // Handle for LPI2C RTOS driver.
  lpi2c_rtos_handle_t controller_handle;
  // Configuration for LPI2C in controller mode.
  lpi2c_master_config_t controller_config;
  // Configuration for LPI2C in target mode.
  lpi2c_target_config_t target_config;
  // Handle for LPI2C driver in target mode.
  lpi2c_target_handle_t target_handle;
  // Callback function for sending data in target mode.
  // Note: this will be called in interrupt context, so this can't take
  // too long to execute.
  std::function<void(I2cConfig*, lpi2c_target_transfer_t*)> target_callback;
  // Root clock frequency of the I2C module.
  uint32_t clk_freq;
};
using TargetCallback =
    std::function<void(I2cConfig*, lpi2c_target_transfer_t*)>;

// I2C buses available on the header pins.
enum class I2c {
  // I2C1_SCL (J10, pin 11) and I2C1_SDA (J10, pin 12)
  kI2c1 = 1,
  // I2C6_SCL (J9, pin 11) and I2C6_SDA (J10, pin 10)
  kI2c6 = 6,
};

// Gets the default configuration for an I2C bus that is available on the
// header.
//
// @param bus `I2c` of the desired bus.
// @returns Configuration for using `bus`.
I2cConfig I2cGetDefaultConfig(I2c bus);

// Initializes a bus in 'controller' mode using the given config.
//
// @param config `I2cConfig` to initialize the hardware with.
bool I2cInitController(I2cConfig& config);

// Initializes a bus in 'target' mode using the given config.
//
// @param config `I2cConfig` to initialize the hardware with.
// @param address Address to listen for on the bus.
// @param callback `TargetCallback` method to provide the response data.
bool I2cInitTarget(I2cConfig& config, uint8_t address, TargetCallback callback);

// Reads data from the configured bus, in `controller` mode.
//
// @param config `I2cConfig` for the configured bus.
// @param address Address of the target device we wish to read from.
// @param buffer Pointer to a buffer that will contain the receieved data.
// @param count Number of bytes to read.
bool I2cControllerRead(I2cConfig& config, uint8_t address, uint8_t* buffer,
                       size_t count);

// Writes data on the configured bus, in `controller` mode.
//
// @param config `I2cConfig` for the configured bus.
// @param address Address of the target device we wish to write to.
// @param buffer Pointer to a buffer that contains the data.
// @param count Number of bytes to write.
bool I2cControllerWrite(I2cConfig& config, uint8_t address, uint8_t* buffer,
                        size_t count);

}  // namespace coralmicro

#endif  // LIBS_BASE_I2C_H_
