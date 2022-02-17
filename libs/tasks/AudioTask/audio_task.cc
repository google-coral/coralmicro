#include "libs/tasks/AudioTask/audio_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.h"

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"

#include <algorithm>

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

void AudioTask::StaticPdmCallback(PDM_Type *base, pdm_edma_handle_t *handle,
                                  status_t status, void *userData) {
    static_cast<AudioTask*>(userData)->PdmCallback(base, handle, status);
}

void AudioTask::PdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status) {
    auto& pdm_transfer = pdm_transfers_[pdm_transfer_index_];

    callback_(callback_param_,
              const_cast<int32_t*>(reinterpret_cast<volatile int32_t*>(pdm_transfer.data)),
              pdm_transfer.dataSize / sizeof(int32_t));

    pdm_transfer_index_ = (pdm_transfer_index_ + 1) % pdm_transfer_count_;

    __DSB();
}

void AudioTask::TaskInit() {
    // TODO(atv): Make a header with DMA MUX configs so we don't end up with
    // collisions down the line
    DMAMUX_Init(DMAMUX0);
    DMAMUX_SetSource(DMAMUX0, kDmaChannel, kDmaRequestMuxPdm);
    DMAMUX_EnableChannel(DMAMUX0, kDmaChannel);

    // TODO(atv): Make a header with these priorities
    NVIC_SetPriority(DMA0_DMA16_IRQn, 6);

    EDMA_GetDefaultConfig(&edma_config_);
    EDMA_Init(DMA0, &edma_config_);
    EDMA_CreateHandle(&edma_handle_, DMA0, kDmaChannel);

    pdm_config_t pdm_config{};
    pdm_config.enableDoze = false;
    pdm_config.fifoWatermark = 4;
    pdm_config.qualityMode = kPDM_QualityModeHigh;
    pdm_config.cicOverSampleRate = 0;
    PDM_Init(PDM, &pdm_config);
    PDM_TransferCreateHandleEDMA(PDM, &pdm_edma_handle_,
                                 AudioTask::StaticPdmCallback, this,
                                 &edma_handle_);

    // TODO(atv): Evaluate, taken from sample
    pdm_channel_config_t channel_config{};
    channel_config.cutOffFreq = kPDM_DcRemoverCutOff152Hz;
    channel_config.gain = kPDM_DfOutputGain5;
    PDM_TransferSetChannelConfigEDMA(PDM, &pdm_edma_handle_, /*channel=*/0,
                                     &channel_config);

    PmicTask::GetSingleton()->SetRailState(pmic::Rail::MIC_1V8, false);
}

void AudioTask::RequestHandler(Request *req) {
    Response resp;
    resp.type = req->type;
    switch (req->type) {
        case RequestType::Power:
            HandlePowerRequest(req->request.power);
            break;
        case RequestType::Enable:
            HandleEnableRequest(req->request.enable);
            break;
        case RequestType::Disable:
            HandleDisableRequest();
            break;
    }
    if (req->callback)
        req->callback(resp);
}

void AudioTask::HandleEnableRequest(const EnableRequest& request) {
    auto status = PDM_SetSampleRateConfig(PDM, kPdmClock,
        static_cast<uint32_t>(request.sample_rate));
    if (status != kStatus_Success) {
        // TODO(dkovalev): implement proper error handling.
        printf("PDM_SetSampleRateConfig() failed.");
    }
    PDM_Reset(PDM);
    PDM_EnableInterrupts(PDM, kPDM_ErrorInterruptEnable);
    EnableIRQ(PDM_ERROR_IRQn);

    pdm_transfer_index_ = 0;
    pdm_transfer_count_ = request.num_buffers;

    PDM_TransferInstallEDMATCDMemory(&pdm_edma_handle_, edma_tcd_, pdm_transfer_count_);

    for (int i = 0; i < pdm_transfer_count_; ++i) {
        pdm_transfers_[i].data = reinterpret_cast<uint8_t*>(request.buffers[i]);
        pdm_transfers_[i].dataSize = request.buffer_size * sizeof(int32_t);
        pdm_transfers_[i].linkTransfer = &pdm_transfers_[(i + 1) % pdm_transfer_count_];
    }

    callback_param_ = request.callback_param;
    callback_ = request.callback;

    status = PDM_TransferReceiveEDMA(PDM, &pdm_edma_handle_, pdm_transfers_);
    if (status != kStatus_Success) {
        // TODO(dkovalev): implement proper error handling.
        printf("PDM_TransferReceiveEDMA() failed.");
    }
}

void AudioTask::HandleDisableRequest() {
    DisableIRQ(PDM_ERROR_IRQn);
    PDM_TransferTerminateReceiveEDMA(PDM, &pdm_edma_handle_);
}

void AudioTask::HandlePowerRequest(const PowerRequest& request) {
    PmicTask::GetSingleton()->SetRailState(pmic::Rail::MIC_1V8, request.enable);
}

void AudioTask::SetPower(bool enable) {
    Request req;
    req.type = RequestType::Power;
    req.request.power.enable = enable;
    SendRequest(req);
}

void AudioTask::Enable(
    audio::SampleRate sample_rate,
    const std::vector<int32_t*>& buffers, size_t buffer_size,
    void *callback_param, audio::AudioTaskCallback callback) {
    Request req;
    req.type = RequestType::Enable;
    req.request.enable.sample_rate = sample_rate;
    std::copy_n(buffers.data(), buffers.size(), req.request.enable.buffers);
    req.request.enable.num_buffers = buffers.size();
    req.request.enable.buffer_size = buffer_size;
    req.request.enable.callback_param = callback_param;
    req.request.enable.callback = callback;
    SendRequest(req);
}

void AudioTask::Disable() {
    Request req;
    req.type = RequestType::Disable;
    SendRequest(req);
}

}  // namespace valiant
