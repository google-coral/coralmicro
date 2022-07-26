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

#include "libs/base/spi.h"

#include "libs/base/check.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"

namespace coralmicro {

void SpiGetDefaultConfig(SpiConfig* config) {
  config->base = LPSPI6;
  config->interrupt = LPSPI6_IRQn;
  config->cs = {IOMUXC_GPIO_LPSR_09_LPSPI6_PCS0};
  config->clk = {IOMUXC_GPIO_LPSR_10_LPSPI6_SCK};
  config->out = {IOMUXC_GPIO_LPSR_11_LPSPI6_SOUT};
  config->in = {IOMUXC_GPIO_LPSR_12_LPSPI6_SIN};
  config->clk_freq = CLOCK_GetFreqFromObs(CCM_OBS_LPSPI6_CLK_ROOT);
  LPSPI_MasterGetDefaultConfig(&config->config);
}

bool SpiInit(SpiConfig& config) {
  CHECK(config.base);
  CHECK(config.interrupt > 0);
  IOMUXC_SetPinMux(config.cs[0], config.cs[1], config.cs[2], config.cs[3],
                   config.cs[4], 0U);
  IOMUXC_SetPinMux(config.clk[0], config.clk[1], config.clk[2], config.clk[3],
                   config.clk[4], 0U);
  IOMUXC_SetPinMux(config.out[0], config.out[1], config.out[2], config.out[3],
                   config.out[4], 0U);
  IOMUXC_SetPinMux(config.in[0], config.in[1], config.in[2], config.in[3],
                   config.in[4], 0U);
  NVIC_SetPriority(static_cast<IRQn_Type>(config.interrupt), 3);
  return LPSPI_RTOS_Init(&config.handle, config.base, &config.config,
                         config.clk_freq) == kStatus_Success;
}

bool SpiTransfer(SpiConfig& config, uint8_t* tx_data, uint8_t* rx_data,
                 size_t size) {
  CHECK(tx_data);
  lpspi_transfer_t xfer;
  xfer.txData = tx_data;
  xfer.rxData = rx_data;
  xfer.dataSize = size;
  xfer.configFlags = kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;
  return LPSPI_RTOS_Transfer(&config.handle, &xfer) == kStatus_Success;
}

}  // namespace coralmicro