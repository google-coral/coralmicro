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

#ifndef LIBS_AUDIO_AUDIO_DRIVER_H_
#define LIBS_AUDIO_AUDIO_DRIVER_H_

#include <cstdint>
#include <optional>

#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.h"

namespace coralmicro {

// Audio sample rates that can be handled by the audio driver.
enum class AudioSampleRate : int32_t {
  // 16000 Hz sample rate.
  k16000_Hz = 16000,
  // 48000 Hz sample rate.
  k48000_Hz = 48000,
};

// Converts a given sample rate to its corresponding AudioSampleRate.
//
// @param sample_rate_hz An int to convert to an AudioSampleRate.
// @return The corresponding sample rate from AudioSampleRate,
// Otherwise returns a std::nullopt.
std::optional<AudioSampleRate> CheckSampleRate(int sample_rate_hz);

// Converts time duration in ms to the number of samples according to specified
// sample rate.
//
// @param sample_rate Conversion rate for 1 second of audio to samples.
// @param ms Amount of time in milliseconds to convert.
// @return Number of samples by sample_rate in a timespan of ms milliseconds.
inline int MsToSamples(AudioSampleRate sample_rate, int ms) {
  return ms * static_cast<int>(sample_rate) / 1000;
}

// Audio driver configuration parameters.
//
// This is required to instantiate `AudioReader` and `AudioService`.
struct AudioDriverConfig {
  // Constructs an AudioDriverConfig.
  //
  // @param sample_rate The sample rate.
  // @param dma_buffer The number of dma buffers to use.
  // @param dma_buffer_ms length in milliseconds of audio data to store in each
  // buffer.
  AudioDriverConfig(AudioSampleRate sample_rate, size_t dma_buffers,
                    size_t dma_buffer_ms)
      : sample_rate{sample_rate},
        num_dma_buffers{dma_buffers},
        dma_buffer_size_ms{dma_buffer_ms} {}

  // Sample rate to be used.
  AudioSampleRate sample_rate;
  // Number of DMA buffers to use.
  size_t num_dma_buffers;
  // Length in milliseconds of audio data to store in each DMA buffer.
  size_t dma_buffer_size_ms;

  // Gets the DMA buffer size in audio samples according to the specified sample
  // rate for the dma_buffersize_ms used in the config.
  //
  // @return The DMA buffer size in audio samples according to the specified
  // sample rate.
  size_t dma_buffer_size_samples() const {
    return MsToSamples(sample_rate, dma_buffer_size_ms);
  }
};

// Tracks the total space allocated for `AudioDriver`.
template <size_t NumDmaBuffers, size_t CombinedDmaBufferSize>
struct AudioDriverBuffers {
  // Total number of DMA buffers allocated.
  static constexpr size_t kNumDmaBuffers = NumDmaBuffers;
  // Total space of all for DMA buffers allocated.
  static constexpr size_t kCombinedDmaBufferSize = CombinedDmaBufferSize;
  // Checks if the allocated space can handle a specific `AudioDriverConfig`.
  //
  // @param config The config to verify.
  // @return bool True if we have enough space to allocate the config,
  // false otherwise.
  static bool CanHandle(const AudioDriverConfig& config) {
    return config.num_dma_buffers <= kNumDmaBuffers &&
           config.num_dma_buffers * config.dma_buffer_size_samples() <=
               kCombinedDmaBufferSize;
  }
  // @cond
  alignas(16) int32_t dma_buffer[kCombinedDmaBufferSize];
  alignas(32) edma_tcd_t edma_tcd[kNumDmaBuffers];
  pdm_edma_transfer_t pdm_transfers[kNumDmaBuffers];
  // @endcond
};

// Provides low-level access to the board's microphone with audio provided by a
// callback function. The callback is called from an interrupt service routine
// (ISR) context and receives audio samples using direct memory access (DMA).
//
// An instance of this class is required for `AudioReader` and `AudioService`,
// but you do not need to call `Enable()` and `Disable()` when using those APIs.
//
// So unless you're building a custom audio service to manage the `AudioDriver`
// lifecycle, you only need to instantiate the `AudioDriver`
// and then pass it to either `AudioReader` or `AudioService`.
//
// For example usage, see `AudioService`.
class AudioDriver {
 public:
  // Callback function type to receive audio samples.
  // Called directly by the ISR.
  //
  // @param ctx Extra parameters for the callback function.
  // @param dma_buffer A pointer to the buffer.
  // @param dma_buffer_size The number of audio samples in the buffer.
  using Callback = void (*)(void* ctx, const int32_t* dma_buffer,
                            size_t dma_buffer_size);

  // Constructor.
  //
  // @param buffers Defines the buffer's total memory capacity.
  template <size_t NumDmaBuffers, size_t CombinedDmaBufferSize>
  explicit AudioDriver(
      AudioDriverBuffers<NumDmaBuffers, CombinedDmaBufferSize>& buffers)
      : dma_buffer_(buffers.dma_buffer),
        combined_dma_buffer_size_(CombinedDmaBufferSize),
        max_num_dma_buffers_(NumDmaBuffers),
        edma_tcd_(buffers.edma_tcd),
        pdm_transfers_(buffers.pdm_transfers) {}

  // Enables the microphone and specifies a callback to receive audio samples.
  //
  // This turns on the microphone and starts audio sampling, but it is called
  // for you when using `AudioReader` or `AudioService`.
  //
  // @param config Driver configuration such as the sample rate and sample size.
  // Used to check if there is space for the specific `AudioDriver`.
  // @param ctx Extra parameters to pass into the callback.
  // @param fn Callback that receives the audio samples.
  // @return True if the microphone successfully starts, false otherwise.
  bool Enable(const AudioDriverConfig& config, void* ctx, Callback fn);
  // Stops processing of new audio data and turns off microphone.
  void Disable();

 private:
  static void StaticPdmCallback(PDM_Type* base, pdm_edma_handle_t* handle,
                                status_t status, void* user_data) {
    static_cast<AudioDriver*>(user_data)->PdmCallback(base, handle, status);
  }
  void PdmCallback(PDM_Type* base, pdm_edma_handle_t* handle, status_t status);

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

  void* ctx_;
  Callback fn_;
};

}  // namespace coralmicro

#endif  // LIBS_AUDIO_AUDIO_DRIVER_H_
