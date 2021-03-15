#ifndef _LIBS_TASKS_AUDIO_TASK_H_
#define _LIBS_TASKS_AUDIO_TASK_H_

#include "libs/base/tasks_m7.h"
#include "libs/base/queue_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.h"
#include <functional>

namespace valiant {

namespace audio {

enum class AudioRequestType : uint8_t {
    Power,
    Enable,
    Disable,
    SetCallback,
    SetBuffer,
};

struct PowerRequest {
   bool enable;
};

typedef uint32_t* (*AudioTaskCallback)(void *);
struct SetCallbackRequest {
    AudioTaskCallback callback;
    void *callback_param;
};

struct SetBufferRequest {
    uint32_t* buffer;
    size_t bytes;
};

struct AudioResponse {
    AudioRequestType type;
    union {
    } response;
};

struct AudioRequest {
    AudioRequestType type;
    union {
        PowerRequest power;
        SetCallbackRequest set_callback;
        SetBufferRequest set_buffer;
    } request;
    std::function<void(AudioResponse)> callback;
};

}  // namespace audio

static constexpr size_t kAudioTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kAudioTaskQueueLength = 4;
extern const char kAudioTaskName[];

class AudioTask : public QueueTask<audio::AudioRequest, kAudioTaskName, kAudioTaskStackDepth, AUDIO_TASK_PRIORITY, kAudioTaskQueueLength> {
  public:
    void Init() override;
    static AudioTask* GetSingleton() {
        static AudioTask audio;
        return &audio;
    }
    void SetPower(bool enable);
    void Enable();
    void Disable();
    void SetCallback(audio::AudioTaskCallback cb, void *param);
    void SetBuffer(uint32_t* buffer, size_t bytes);
  private:
    static void StaticPdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData);
    void PdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status);
    void TaskInit() override;
    audio::AudioResponse SendRequest(audio::AudioRequest& req);
    void MessageHandler(audio::AudioRequest *req) override;
    void HandlePowerRequest(audio::PowerRequest& power);
    void HandleEnableRequest();
    void HandleDisableRequest();
    void HandleSetCallback(audio::SetCallbackRequest& set_callback);
    void HandleSetBuffer(audio::SetBufferRequest& set_buffer);

    edma_config_t edma_config_;
    // TODO(atv): marked noncacheable in demo. do i care?
    edma_handle_t edma_handle_ __attribute__((aligned(4)));
    pdm_config_t pdm_config_;
    pdm_edma_handle_t pdm_edma_handle_ __attribute__((aligned(4)));
    edma_tcd_t edma_tcd_ __attribute__((aligned(32)));
    pdm_channel_config_t channel_config_;
    pdm_edma_transfer_t pdm_transfer_;

    audio::AudioTaskCallback callback_ = nullptr;
    void* callback_param_;
    uint32_t *rx_buffer_;
    size_t rx_buffer_bytes_;
    // uint16_t temp_rx_buffer_[512];
};

}  // namespace valiant

#endif  // _LIBS_TASKS_AUDIO_TASK_H_
