#ifndef _LIBS_TASKS_AUDIO_TASK_H_
#define _LIBS_TASKS_AUDIO_TASK_H_

#include "libs/base/tasks.h"
#include "libs/base/queue_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.h"
#include <functional>
#include <vector>

namespace valiant {
namespace audio {

enum class RequestType : uint8_t {
    Power,
    Enable,
    Disable,
};

struct PowerRequest {
   bool enable;
};

enum class SampleRate : uint32_t {
    k16000_Hz = 16000,
    k48000_Hz = 48000,
};

using AudioTaskCallback =
    void (*)(void *param, const int32_t* buffer, size_t buffer_size);

constexpr size_t kMaxNumBuffers = 16;

struct EnableRequest {
   SampleRate sample_rate;

   int32_t* buffers[kMaxNumBuffers];
   size_t num_buffers;
   size_t buffer_size;

   void *callback_param;
   AudioTaskCallback callback;
};

struct Response {
    RequestType type;
    union {
    } response;
};

struct Request {
    RequestType type;
    union {
        PowerRequest power;
        EnableRequest enable;
    } request;
    std::function<void(Response)> callback;
};

}  // namespace audio

static constexpr size_t kAudioTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kAudioTaskQueueLength = 4;
extern const char kAudioTaskName[];

class AudioTask : public QueueTask<audio::Request,
                                   audio::Response,
                                   kAudioTaskName,
                                   kAudioTaskStackDepth,
                                   AUDIO_TASK_PRIORITY,
                                   kAudioTaskQueueLength> {
  public:
    static AudioTask* GetSingleton() {
        static AudioTask audio;
        return &audio;
    }
    void SetPower(bool enable);
    void Enable(audio::SampleRate sample_rate,
                const std::vector<int32_t*>& buffers, size_t buffer_size,
                void *callback_param, audio::AudioTaskCallback callback);
    void Disable();

  private:
    static void StaticPdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData);
    void PdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status);

    void TaskInit() override;

    void RequestHandler(audio::Request *req) override;
    void HandlePowerRequest(const audio::PowerRequest& request);
    void HandleEnableRequest(const audio::EnableRequest& request);
    void HandleDisableRequest();

  private:
    edma_config_t edma_config_;
    // TODO(atv): marked noncacheable in demo. do i care?
    edma_handle_t edma_handle_ __attribute__((aligned(4)));

    pdm_edma_handle_t pdm_edma_handle_ __attribute__((aligned(4)));
    edma_tcd_t edma_tcd_[audio::kMaxNumBuffers] __attribute__((aligned(32)));

    pdm_edma_transfer_t pdm_transfers_[audio::kMaxNumBuffers];
    volatile int pdm_transfer_index_;
    size_t pdm_transfer_count_;

    void* callback_param_ = nullptr;
    audio::AudioTaskCallback callback_ = nullptr;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_AUDIO_TASK_H_
