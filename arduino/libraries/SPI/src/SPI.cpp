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

#include "SPI.h"

#include <cassert>

#include "libs/base/tasks.h"
extern "C" uint32_t LPSPI_GetInstance(LPSPI_Type *);
static IRQn_Type const kLpspiIrqs[] = LPSPI_IRQS;

namespace coralmicro {
namespace arduino {

HardwareSPI::HardwareSPI(LPSPI_Type *base) : base_(base) {}

void HardwareSPI::begin() {
  if (!initialized_) {
    uint32_t instance = LPSPI_GetInstance(base_);
    NVIC_SetPriority(kLpspiIrqs[instance], kInterruptPriority);
    LPSPI_MasterGetDefaultConfig(&config_);
    status_t status =
        LPSPI_RTOS_Init(&handle_, base_, &config_,
                        CLOCK_GetFreqFromObs(CCM_OBS_LPSPI6_CLK_ROOT));
    if (status != kStatus_Success) {
      printf("Init failed: %d\n", status);
      return;
    }
    initialized_ = true;
  }
}

void HardwareSPI::end() {
  if (initialized_) {
    status_t status = LPSPI_RTOS_Deinit(&handle_);
    if (status != kStatus_Success) {
      printf("Deinit failed: %d\n", status);
    }
    initialized_ = false;
  }
}

uint32_t HardwareSPI::GetConfigFlags() {
  return (config_.whichPcs << LPSPI_MASTER_PCS_SHIFT) |
         kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;
}

void HardwareSPI::updateSettings(::arduino::SPISettings settings) {
  config_.direction = (settings.getBitOrder() == BitOrder::LSBFIRST)
                          ? kLPSPI_LsbFirst
                          : kLPSPI_MsbFirst;
  config_.cpha = (settings.getDataMode() % 2 == 0)
                     ? kLPSPI_ClockPhaseFirstEdge
                     : kLPSPI_ClockPhaseSecondEdge;
  config_.cpol = (settings.getDataMode() / 2 == 0)
                     ? kLPSPI_ClockPolarityActiveHigh
                     : kLPSPI_ClockPolarityActiveLow;
  config_.whichPcs = (lpspi_which_pcs_t)(settings.getDataMode());

  // Applies the new config values
  LPSPI_MasterInit(base_, &config_, settings.getClockFreq());
}

void HardwareSPI::beginTransaction(::arduino::SPISettings settings) {}

void HardwareSPI::endTransaction(void) {
  // endTransaction is not supposed to reset the settings, so LPSPI_RTOS_Deinit
  // is not called here
}

uint8_t HardwareSPI::transfer(uint8_t data) {
  uint8_t result = 0U;
  lpspi_transfer_t masterXfer;
  masterXfer.txData = &data;
  masterXfer.rxData = &result;
  masterXfer.dataSize = 1U;
  masterXfer.configFlags = GetConfigFlags();

  status_t status = LPSPI_RTOS_Transfer(&handle_, &masterXfer);
  if (status != kStatus_Success) {
    printf("Transfer failed: %d\n", status);
  }

  return result;
}

uint16_t HardwareSPI::transfer16(uint16_t data) {
  uint8_t *txBuf = (uint8_t *)(&data);
  uint8_t result[2] = {0U};

  lpspi_transfer_t masterXfer;
  masterXfer.txData = txBuf;
  masterXfer.rxData = result;
  masterXfer.dataSize = 2U;
  masterXfer.configFlags = GetConfigFlags();

  status_t status = LPSPI_RTOS_Transfer(&handle_, &masterXfer);
  if (status != kStatus_Success) {
    printf("Transfer failed: %d\n", status);
  }

  return result[0] + (result[1] << 8);
}

void HardwareSPI::transfer(void *buf, size_t count) {
  if (count == 0) {
    return;
  }

  uint8_t *txBuf = (uint8_t *)buf;

  lpspi_transfer_t masterXfer;
  masterXfer.txData = txBuf;
  masterXfer.rxData = txBuf;
  masterXfer.dataSize = count;
  masterXfer.configFlags = GetConfigFlags();

  status_t status = LPSPI_RTOS_Transfer(&handle_, &masterXfer);
  if (status != kStatus_Success) {
    printf("Transfer failed: %d\n", status);
  }
}

void HardwareSPI::usingInterrupt(int interruptNumber) {
  // Not Implemented
}

void HardwareSPI::notUsingInterrupt(int interruptNumber) {
  // Not Implemented
}

void HardwareSPI::attachInterrupt() {
  // Not Implemented
}

void HardwareSPI::detachInterrupt() {
  // Not Implemented
}

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::HardwareSPI SPI(LPSPI6);
