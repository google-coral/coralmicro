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

#include "libs/audio/audio_driver.h"

#include "libs/pmic/pmic.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.h"

extern "C" void PDM_ERROR_IRQHandler() {
  uint32_t status = 0;
  status = PDM_GetStatus(PDM);
  if (status & PDM_STAT_LOWFREQF_MASK) {
    PDM_ClearStatus(PDM, PDM_STAT_LOWFREQF_MASK);
  }

  status = PDM_GetFifoStatus(PDM);
  if (status) {
    PDM_ClearFIFOStatus(PDM, status);
  }

  status = PDM_GetRangeStatus(PDM);
  if (status) {
    PDM_ClearRangeStatus(PDM, status);
  }

  __DSB();
}

namespace coralmicro {
namespace {
constexpr int kDmaChannel = 0;
constexpr int kPdmClock = 24577500;
}  // namespace

std::optional<AudioSampleRate> CheckSampleRate(int sample_rate_hz) {
  switch (sample_rate_hz) {
    case 16000:
      return AudioSampleRate::k16000_Hz;
    case 48000:
      return AudioSampleRate::k48000_Hz;
  }
  return std::nullopt;
}

void AudioDriver::PdmCallback(PDM_Type* base, pdm_edma_handle_t* handle,
                              status_t status) {
  auto& pdm_transfer = pdm_transfers_[pdm_transfer_index_];

  fn_(ctx_,
      const_cast<int32_t*>(
          reinterpret_cast<volatile int32_t*>(pdm_transfer.data)),
      pdm_transfer.dataSize / sizeof(int32_t));

  pdm_transfer_index_ = (pdm_transfer_index_ + 1) % pdm_transfer_count_;

  __DSB();
}

bool AudioDriver::Enable(const AudioDriverConfig& config, void* ctx,
                         Callback fn) {
  if (config.num_dma_buffers > max_num_dma_buffers_) {
    printf("ERROR: Too many DMA buffers.\r\n");
    return false;
  }

  const auto dma_buffer_size = config.dma_buffer_size_samples();
  if (config.num_dma_buffers * dma_buffer_size > combined_dma_buffer_size_) {
    printf("ERROR: Not enough DMA memory.\r\n");
    return false;
  }

  pdm_transfer_index_ = 0;
  pdm_transfer_count_ = config.num_dma_buffers;
  ctx_ = ctx;
  fn_ = fn;

  PmicTask::GetSingleton()->SetRailState(PmicRail::kMic1V8, true);

  // TODO(atv): Make a header with DMA MUX configs so we don't end up with
  // collisions down the line
  DMAMUX_Init(DMAMUX0);
  DMAMUX_SetSource(DMAMUX0, kDmaChannel, kDmaRequestMuxPdm);
  DMAMUX_EnableChannel(DMAMUX0, kDmaChannel);

  // TODO(atv): Make a header with these priorities
  NVIC_SetPriority(DMA0_DMA16_IRQn, 6);

  edma_config_t edma_config{};
  EDMA_GetDefaultConfig(&edma_config);
  EDMA_Init(DMA0, &edma_config);
  EDMA_CreateHandle(&edma_handle_, DMA0, kDmaChannel);

  pdm_config_t pdm_config{};
  pdm_config.enableDoze = false;
  pdm_config.fifoWatermark = 4;
  pdm_config.qualityMode = kPDM_QualityModeHigh;
  pdm_config.cicOverSampleRate = 0;
  PDM_Init(PDM, &pdm_config);
  PDM_TransferCreateHandleEDMA(PDM, &pdm_edma_handle_, StaticPdmCallback, this,
                               &edma_handle_);

  // TODO(atv): Evaluate, taken from sample
  pdm_channel_config_t channel_config{};
  channel_config.cutOffFreq = kPDM_DcRemoverCutOff152Hz;
  channel_config.gain = kPDM_DfOutputGain5;
  PDM_TransferSetChannelConfigEDMA(PDM, &pdm_edma_handle_, /*channel=*/0,
                                   &channel_config);

  auto status = PDM_SetSampleRateConfig(
      PDM, kPdmClock, static_cast<int32_t>(config.sample_rate));
  if (status != kStatus_Success) {
    // TODO(dkovalev): implement proper error handling.
    printf("ERROR: PDM_SetSampleRateConfig() failed.\r\n");
    return false;
  }
  PDM_Reset(PDM);
  PDM_EnableInterrupts(PDM, kPDM_ErrorInterruptEnable);
  EnableIRQ(PDM_ERROR_IRQn);

  PDM_TransferInstallEDMATCDMemory(&pdm_edma_handle_, edma_tcd_,
                                   pdm_transfer_count_);

  for (size_t i = 0; i < pdm_transfer_count_; ++i) {
    pdm_transfers_[i].data =
        reinterpret_cast<uint8_t*>(dma_buffer_ + i * dma_buffer_size);
    pdm_transfers_[i].dataSize = dma_buffer_size * sizeof(int32_t);
    pdm_transfers_[i].linkTransfer =
        &pdm_transfers_[(i + 1) % pdm_transfer_count_];
  }

  status = PDM_TransferReceiveEDMA(PDM, &pdm_edma_handle_, pdm_transfers_);
  if (status != kStatus_Success) {
    // TODO(dkovalev): implement proper error handling.
    printf("ERROR: PDM_TransferReceiveEDMA() failed.\r\n");
    return false;
  }

  return true;
}

void AudioDriver::Disable() {
  DisableIRQ(PDM_ERROR_IRQn);
  PDM_TransferTerminateReceiveEDMA(PDM, &pdm_edma_handle_);
  PDM_Deinit(PDM);
  DMAMUX_Deinit(DMAMUX0);

  PmicTask::GetSingleton()->SetRailState(PmicRail::kMic1V8, false);
}

}  // namespace coralmicro
