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

#ifndef LIBS_BASE_SPI_H_
#define LIBS_BASE_SPI_H_

#include <array>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi_freertos.h"

namespace coralmicro {

// Configuration for a SPI interface.
struct SpiConfig {
  // Base pointer to the SPI peripheral.
  LPSPI_Type* base;
  // Interrupt number for the peripheral.
  int interrupt;
  // IOMUXC configuration for CS pin.
  std::array<uint32_t, 5> cs;
  // IOMUXC configuration for SDO pin.
  std::array<uint32_t, 5> out;
  // IOMUXC configuration for SDI pin.
  std::array<uint32_t, 5> in;
  // IOMUXC configuration for SCK pin.
  std::array<uint32_t, 5> clk;
  // Handle for LPSPI RTOS driver.
  lpspi_rtos_handle_t handle;
  // Config for LPSPI module.
  lpspi_master_config_t config;
  // Output clock frequency.
  uint32_t clk_freq;
};

// Gets the default configuration for using SPI on the device header.
//
// @param config `SpiConfig` to populate with default values.
void SpiGetDefaultConfig(SpiConfig* config);

// Initializes SPI with a given configuration.
//
// @param config `SpiConfig` to initialize hardware with.
// @returns True on success; false otherwise.
bool SpiInit(SpiConfig& config);

// Executes a transfer over SPI with the given configuration.
//
// @param config `SpiConfig` to use to execute the transaction.
// @param tx_data Pointer to data that will be sent.
// @param rx_data Pointer to a buffer to contain received data (may be NULL).
// @param size Number of bytes to transfer.
// @returns True on success; false otherwise.
bool SpiTransfer(SpiConfig& config, uint8_t* tx_data, uint8_t* rx_data,
                 size_t size);

}  // namespace coralmicro

#endif  // LIBS_BASE_SPI_H_