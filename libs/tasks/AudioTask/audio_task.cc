#include "libs/tasks/AudioTask/audio_task.h"

#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.h"

extern "C" void PDM_ERROR_IRQHandler(void) {
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

namespace valiant {
namespace {
constexpr int kDmaChannel = 0;
constexpr int kPdmClock = 24577500;
}  // namespace

using namespace audio;

constexpr const char kAudioTaskName[] = "audio_task";

std::optional<audio::SampleRate> CheckSampleRate(int sample_rate_hz) {
    switch (sample_rate_hz) {
        case 16000:
            return audio::SampleRate::k16000_Hz;
        case 48000:
            return audio::SampleRate::k48000_Hz;
    }
    return std::nullopt;
}

void AudioDriver::PdmCallback(PDM_Type* base, pdm_edma_handle_t* handle,
                              status_t status) {
    auto& pdm_transfer = pdm_transfers_[pdm_transfer_index_];

    callback_(callback_param_,
              const_cast<int32_t*>(
                  reinterpret_cast<volatile int32_t*>(pdm_transfer.data)),
              pdm_transfer.dataSize / sizeof(int32_t));

    pdm_transfer_index_ = (pdm_transfer_index_ + 1) % pdm_transfer_count_;

    __DSB();
}

bool AudioDriver::Enable(audio::SampleRate sample_rate, int32_t** dma_buffers,
                         size_t num_dma_buffers, size_t dma_buffer_size,
                         void* callback_param,
                         audio::AudioTaskCallback callback) {
    PmicTask::GetSingleton()->SetRailState(pmic::Rail::MIC_1V8, true);

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
    PDM_TransferCreateHandleEDMA(PDM, &pdm_edma_handle_, StaticPdmCallback,
                                 this, &edma_handle_);

    // TODO(atv): Evaluate, taken from sample
    pdm_channel_config_t channel_config{};
    channel_config.cutOffFreq = kPDM_DcRemoverCutOff152Hz;
    channel_config.gain = kPDM_DfOutputGain5;
    PDM_TransferSetChannelConfigEDMA(PDM, &pdm_edma_handle_, /*channel=*/0,
                                     &channel_config);

    auto status = PDM_SetSampleRateConfig(PDM, kPdmClock,
                                          static_cast<uint32_t>(sample_rate));
    if (status != kStatus_Success) {
        // TODO(dkovalev): implement proper error handling.
        printf("ERROR: PDM_SetSampleRateConfig() failed.\n\r");
        return false;
    }
    PDM_Reset(PDM);
    PDM_EnableInterrupts(PDM, kPDM_ErrorInterruptEnable);
    EnableIRQ(PDM_ERROR_IRQn);

    pdm_transfer_index_ = 0;
    pdm_transfer_count_ = num_dma_buffers;

    PDM_TransferInstallEDMATCDMemory(&pdm_edma_handle_, edma_tcd_,
                                     pdm_transfer_count_);

    for (size_t i = 0; i < pdm_transfer_count_; ++i) {
        pdm_transfers_[i].data = reinterpret_cast<uint8_t*>(dma_buffers[i]);
        pdm_transfers_[i].dataSize = dma_buffer_size * sizeof(int32_t);
        pdm_transfers_[i].linkTransfer =
            &pdm_transfers_[(i + 1) % pdm_transfer_count_];
    }

    callback_param_ = callback_param;
    callback_ = callback;

    status = PDM_TransferReceiveEDMA(PDM, &pdm_edma_handle_, pdm_transfers_);
    if (status != kStatus_Success) {
        // TODO(dkovalev): implement proper error handling.
        printf("ERROR: PDM_TransferReceiveEDMA() failed.\n\r");
        return false;
    }

    return true;
}

void AudioDriver::Disable() {
    DisableIRQ(PDM_ERROR_IRQn);
    PDM_TransferTerminateReceiveEDMA(PDM, &pdm_edma_handle_);
    PDM_Deinit(PDM);
    DMAMUX_Deinit(DMAMUX0);

    PmicTask::GetSingleton()->SetRailState(pmic::Rail::MIC_1V8, false);
}

void AudioTask::Enable(audio::SampleRate sample_rate, int32_t** dma_buffers,
                       size_t num_dma_buffers, size_t dma_buffer_size,
                       void* callback_param,
                       audio::AudioTaskCallback callback) {
    Request req;
    req.type = RequestType::Enable;
    req.request.enable.sample_rate = sample_rate;
    req.request.enable.dma_buffers = dma_buffers;
    req.request.enable.num_dma_buffers = num_dma_buffers;
    req.request.enable.dma_buffer_size = dma_buffer_size;
    req.request.enable.callback_param = callback_param;
    req.request.enable.callback = callback;
    SendRequest(req);
}

void AudioTask::Disable() {
    Request req;
    req.type = RequestType::Disable;
    SendRequest(req);
}

void AudioTask::RequestHandler(Request* req) {
    switch (req->type) {
        case RequestType::Enable: {
            auto& r = req->request.enable;
            driver_.Enable(r.sample_rate, r.dma_buffers, r.num_dma_buffers,
                           r.dma_buffer_size, r.callback_param, r.callback);
        } break;
        case RequestType::Disable:
            driver_.Disable();
            break;
    }
}

}  // namespace valiant
