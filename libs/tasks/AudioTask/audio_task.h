#ifndef _LIBS_TASKS_AUDIO_TASK_H_
#define _LIBS_TASKS_AUDIO_TASK_H_

#include <cstdint>
#include <optional>

#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.h"

namespace valiant {

enum class AudioSampleRate : int32_t {
    k16000_Hz = 16000,
    k48000_Hz = 48000,
};

std::optional<AudioSampleRate> CheckSampleRate(int sample_rate_hz);

inline int MsToSamples(AudioSampleRate sample_rate, int ms) {
    return ms * static_cast<int>(sample_rate) / 1000;
}

struct AudioDriverConfig {
    AudioSampleRate sample_rate;
    size_t num_dma_buffers;
    size_t dma_buffer_size_ms;

    size_t dma_buffer_size_samples() const {
        return MsToSamples(sample_rate, dma_buffer_size_ms);
    }
};

template <size_t NumDmaBuffers, size_t CombinedDmaBufferSize>
struct AudioDriverBuffers {
    static constexpr size_t kNumDmaBuffers = NumDmaBuffers;
    static constexpr size_t kCombinedDmaBufferSize = CombinedDmaBufferSize;

    static bool CanHandle(const AudioDriverConfig& config) {
        return config.num_dma_buffers <= kNumDmaBuffers &&
               config.num_dma_buffers * config.dma_buffer_size_samples() <=
                   kCombinedDmaBufferSize;
    }

    alignas(16) int32_t dma_buffer[kCombinedDmaBufferSize];
    alignas(32) edma_tcd_t edma_tcd[kNumDmaBuffers];
    pdm_edma_transfer_t pdm_transfers[kNumDmaBuffers];
};

class AudioDriver {
   public:
    using Callback = void (*)(void* param, const int32_t* dma_buffer,
                              size_t dma_buffer_size);

    template <size_t NumDmaBuffers, size_t CombinedDmaBufferSize>
    AudioDriver(
        AudioDriverBuffers<NumDmaBuffers, CombinedDmaBufferSize>& buffers)
        : dma_buffer_(buffers.dma_buffer),
          combined_dma_buffer_size_(CombinedDmaBufferSize),
          max_num_dma_buffers_(NumDmaBuffers),
          edma_tcd_(buffers.edma_tcd),
          pdm_transfers_(buffers.pdm_transfers) {}

    bool Enable(const AudioDriverConfig& config, void* callback_param,
                Callback callback);

    void Disable();

   private:
    static void StaticPdmCallback(PDM_Type* base, pdm_edma_handle_t* handle,
                                  status_t status, void* userData) {
        static_cast<AudioDriver*>(userData)->PdmCallback(base, handle, status);
    }
    void PdmCallback(PDM_Type* base, pdm_edma_handle_t* handle,
                     status_t status);

   private:
    // Assigned from constructor call.
    int32_t* dma_buffer_;
    size_t combined_dma_buffer_size_;
    size_t max_num_dma_buffers_;
    edma_tcd_t* edma_tcd_;                // `max_num_dma_buffers_` items
    pdm_edma_transfer_t* pdm_transfers_;  // `max_num_dma_buffers_` items

    // Assigned from Enable() call.
    edma_handle_t edma_handle_;
    pdm_edma_handle_t pdm_edma_handle_;

    volatile int pdm_transfer_index_;
    size_t pdm_transfer_count_;

    void* callback_param_;
    Callback callback_;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_AUDIO_TASK_H_
