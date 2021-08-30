#include "libs/tasks/AudioTask/audio_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.h"

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"

namespace valiant {

using namespace audio;

constexpr const char kAudioTaskName[] = "audio_task";

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

void AudioTask::Init() {
    QueueTask::Init();
}

void AudioTask::StaticPdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData) {
    AudioTask *task = static_cast<AudioTask*>(userData);
    task->PdmCallback(base, handle, status);
}

void AudioTask::PdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status) {
    BaseType_t reschedule = pdFALSE;
    if (status == kStatus_PDM_Idle && callback_) {
        uint32_t* next_buffer = callback_(callback_param_);

        if (next_buffer) {
            SetBufferRequest set_buffer;
            set_buffer.buffer = next_buffer;
            set_buffer.bytes = rx_buffer_bytes_;
            HandleSetBuffer(set_buffer);

            Request req;
            req.type = RequestType::Enable;
            req.callback = nullptr;
            xQueueSendFromISR(request_queue_, &req, &reschedule);
        }
    }
    __DSB();
    portYIELD_FROM_ISR(reschedule);
}

void AudioTask::TaskInit() {
    // TODO(atv): Make a header with DMA MUX configs so we don't end up with collisions down the line
    DMAMUX_Init(DMAMUX0);
    DMAMUX_SetSource(DMAMUX0, 0 /* channel */, kDmaRequestMuxPdm);
    DMAMUX_EnableChannel(DMAMUX0, 0 /* channel */);

    EDMA_GetDefaultConfig(&edma_config_);
    EDMA_Init(DMA0, &edma_config_);
    EDMA_CreateHandle(&edma_handle_, DMA0, 0 /* channel */);

    pdm_config_.enableDoze = false;
    pdm_config_.fifoWatermark = 4;
    // TODO(atv): Evaluate , taken from sample
    pdm_config_.qualityMode = kPDM_QualityModeHigh;
    pdm_config_.cicOverSampleRate = 0;
    PDM_Init(PDM, &pdm_config_);
    PDM_TransferCreateHandleEDMA(PDM, &pdm_edma_handle_, AudioTask::StaticPdmCallback, this, &edma_handle_);
    PDM_TransferInstallEDMATCDMemory(&pdm_edma_handle_, &edma_tcd_, 1);

    // TODO(atv): Evaluate, taken from sample
    channel_config_.cutOffFreq = kPDM_DcRemoverCutOff152Hz;
    channel_config_.gain = kPDM_DfOutputGain6;
    PDM_TransferSetChannelConfigEDMA(PDM, &pdm_edma_handle_, 0 /* left */, &channel_config_);
    // TODO(atv): Parametrize sampling rate?
    /* status_t status = */ PDM_SetSampleRateConfig(PDM, 96000000 /* PDM_CLK */, 16000 /* Sample rate */);
    // printf("PDM_SetSampleRateConfig success? %s\r\n", (status == kStatus_Success) ? "yes" : "no");
    PDM_Reset(PDM);
    PDM_EnableInterrupts(PDM, kPDM_ErrorInterruptEnable);
    // TODO(atv): Make a header with these priorities
    NVIC_SetPriority(DMA0_DMA16_IRQn, 6);

    PowerRequest req;
    req.enable = false;
    HandlePowerRequest(req);
}

void AudioTask::HandleEnableRequest() {
    // TODO(atv): How much of TaskInit should move in here?
    // TODO(atv): add disable (test via keyword in microspeech)
    pdm_transfer_.data = reinterpret_cast<uint8_t*>(rx_buffer_);
    pdm_transfer_.dataSize = rx_buffer_bytes_;
    pdm_transfer_.linkTransfer = NULL;
    PDM_TransferReceiveEDMA(PDM, &pdm_edma_handle_, &pdm_transfer_);
}

void AudioTask::HandleDisableRequest() {
    PDM_TransferTerminateReceiveEDMA(PDM, &pdm_edma_handle_);
}

void AudioTask::HandlePowerRequest(PowerRequest& power) {
    PmicTask::GetSingleton()->SetRailState(
            pmic::Rail::MIC_1V8, power.enable);
}

void AudioTask::HandleSetCallback(SetCallbackRequest& set_callback) {
    callback_ = set_callback.callback;
    callback_param_ = set_callback.callback_param;
}

void AudioTask::HandleSetBuffer(SetBufferRequest& set_buffer) {
    rx_buffer_ = set_buffer.buffer;
    rx_buffer_bytes_ = set_buffer.bytes;
}

void AudioTask::RequestHandler(Request *req) {
    Response resp;
    resp.type = req->type;
    switch (req->type) {
        case RequestType::Power:
            HandlePowerRequest(req->request.power);
            break;
        case RequestType::Enable:
            HandleEnableRequest();
            break;
        case RequestType::Disable:
            HandleDisableRequest();
            break;
        case RequestType::SetCallback:
            HandleSetCallback(req->request.set_callback);
            break;
        case RequestType::SetBuffer:
            HandleSetBuffer(req->request.set_buffer);
            break;
    }
    if (req->callback)
        req->callback(resp);
}

void AudioTask::SetPower(bool enable) {
    Request req;
    req.type = RequestType::Power;
    req.request.power.enable = enable;
    SendRequest(req);
}

void AudioTask::Enable() {
    Request req;
    req.type = RequestType::Enable;
    SendRequest(req);
}

void AudioTask::Disable() {
    Request req;
    req.type = RequestType::Disable;
    SendRequest(req);
}

void AudioTask::SetCallback(AudioTaskCallback cb, void *param) {
    Request req;
    req.type = RequestType::SetCallback;
    req.request.set_callback.callback = cb;
    req.request.set_callback.callback_param = param;
    SendRequest(req);
}

void AudioTask::SetBuffer(uint32_t* buffer, size_t bytes) {
    Request req;
    req.type = RequestType::SetBuffer;
    req.request.set_buffer.buffer = buffer;
    req.request.set_buffer.bytes = bytes;
    SendRequest(req);
}

}  // namespace valiant
