#ifndef _LIBS_TASKS_AUDIO_TASK_H_
#define _LIBS_TASKS_AUDIO_TASK_H_

#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.h"

namespace valiant {
namespace audio {

constexpr size_t kMaxNumBuffers = 32;

enum class RequestType : uint8_t {
    Enable,
    Disable,
};

enum class SampleRate : uint32_t {
    k16000_Hz = 16000,
    k48000_Hz = 48000,
};

using AudioTaskCallback = void (*)(void* param, const int32_t* dma_buffer,
                                   size_t dma_buffer_size);

struct EnableRequest {
    SampleRate sample_rate;

    int32_t** dma_buffers;
    size_t num_dma_buffers;
    size_t dma_buffer_size;

    void* callback_param;
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
        EnableRequest enable;
    } request;
    std::function<void(Response)> callback;
};

}  // namespace audio

static constexpr size_t kAudioTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kAudioTaskQueueLength = 4;
extern const char kAudioTaskName[];

class AudioDriver {
   public:
    bool Enable(audio::SampleRate sample_rate, int32_t** dma_buffers,
                size_t num_dma_buffers, size_t dma_buffer_size,
                void* callback_param, audio::AudioTaskCallback callback);
    void Disable();

   private:
    static void StaticPdmCallback(PDM_Type* base, pdm_edma_handle_t* handle,
                                  status_t status, void* userData) {
        static_cast<AudioDriver*>(userData)->PdmCallback(base, handle, status);
    }
    void PdmCallback(PDM_Type* base, pdm_edma_handle_t* handle,
                     status_t status);

   private:
    edma_handle_t edma_handle_;
    pdm_edma_handle_t pdm_edma_handle_;
    alignas(32) edma_tcd_t edma_tcd_[audio::kMaxNumBuffers];

    pdm_edma_transfer_t pdm_transfers_[audio::kMaxNumBuffers];
    volatile int pdm_transfer_index_;
    size_t pdm_transfer_count_;

    void* callback_param_ = nullptr;
    audio::AudioTaskCallback callback_ = nullptr;
};

class AudioTask : public QueueTask<audio::Request, audio::Response,
                                   kAudioTaskName, kAudioTaskStackDepth,
                                   AUDIO_TASK_PRIORITY, kAudioTaskQueueLength> {
   public:
    static AudioTask* GetSingleton() {
        static AudioTask audio;
        return &audio;
    }
    void Enable(audio::SampleRate sample_rate, int32_t** dma_buffers,
                size_t num_dma_buffers, size_t dma_buffer_size,
                void* callback_param, audio::AudioTaskCallback callback);
    void Disable();

   private:
    void RequestHandler(audio::Request* req) override;

    AudioDriver driver_;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_AUDIO_TASK_H_
