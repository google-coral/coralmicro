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

#ifndef LIBS_AUDIO_AUDIO_SERVICE_H_
#define LIBS_AUDIO_AUDIO_SERVICE_H_

#include <algorithm>
#include <cstdint>
#include <vector>

#include "libs/audio/audio_driver.h"
#include "libs/base/mutex.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"

namespace coralmicro {

template <typename T>
// @cond
class FreeRTOSStreamBuffer {
 public:
  FreeRTOSStreamBuffer() : handle_(nullptr) {}

  ~FreeRTOSStreamBuffer() {
    if (handle_) vStreamBufferDelete(handle_);
  }

  bool Ok() const { return handle_ != nullptr; }

  void Create(size_t xBufferSize, size_t xTriggerLevel) {
    handle_ =
        xStreamBufferCreate(xBufferSize * sizeof(T), xTriggerLevel * sizeof(T));
  }

  size_t Send(const T* pvTxData, size_t xDataLength, TickType_t xTicksToWait) {
    return xStreamBufferSend(handle_, pvTxData, xDataLength * sizeof(T),
                             xTicksToWait) /
           sizeof(T);
  }

  size_t SendFromISR(const T* pvTxData, size_t xDataLength,
                     BaseType_t* pxHigherPriorityTaskWoken) {
    return xStreamBufferSendFromISR(handle_, pvTxData, xDataLength * sizeof(T),
                                    pxHigherPriorityTaskWoken) /
           sizeof(T);
  }

  size_t Receive(T* pvRxData, size_t xBufferLength, TickType_t xTicksToWait) {
    return xStreamBufferReceive(handle_, pvRxData, xBufferLength * sizeof(T),
                                xTicksToWait) /
           sizeof(T);
  }

  size_t ReceiveFromISR(void* pvRxData, size_t xBufferLength,
                        BaseType_t* pxHigherPriorityTaskWoken) {
    return xStreamBufferReceiveFromISR(handle_, pvRxData,
                                       xBufferLength * sizeof(T),
                                       pxHigherPriorityTaskWoken) /
           sizeof(T);
  }

  BaseType_t Reset() { return xStreamBufferReset(handle_); }

  BaseType_t IsEmpty() const { return xStreamBufferIsEmpty(handle_); }

  BaseType_t IsFull() const { return xStreamBufferIsFull(handle_); }

 private:
  StreamBufferHandle_t handle_;
};
// @endcond

// Provides a mechanism to read audio samples from the on-board
// microphone on-demand.
//
// `AudioReader` manages an internal ring buffer that copies audio samples
// from the `AudioDriver` you provide to the constructor. You can read samples
// from the ring buffer at any time by calling
// `FillBuffer()`. This moves the samples into a regular buffer provided by
// `Buffer()`, which you can then read for audio processing. Be sure you call
// `FillBuffer()` fast enough to remove the samples from the ring buffer
// and make room for new incoming samples. If you don't, the ring buffer will
// overflow (incrementing `OverflowCount()`) and you'll miss audio data
// (the ring buffer continues to write so you always get the latest audio).
//
// The microphone remains powered as long as the `AudioReader` is in scope;
// it powers off as soon as the `AudioReader` is destroyed.
//
// For example, this code shows how to set up an `AudioReader` and copy audio
// samples into the buffer and then read it:
//
// ```
// namespace {
// AudioDriverBuffers</*NumDmaBuffers=*/4, /*DmaBufferSize=*/6 * 1024>
//     g_audio_buffers;
// AudioDriver g_audio_driver(g_audio_buffers);
// }  // namespace
//
// const AudioDriverConfig config{audio::SampleRate::k16000_Hz,
//                                /*num_dma_buffers=*/4,
//                                /*dma_buffer_size_ms=*/30};
// AudioReader reader(&g_audio_driver, config);
//
// auto& buffer = reader.Buffer();
// while (true) {
//     auto size = reader.FillBuffer();
//     ProcessBuffer(buffer.data(), size);
// }
// ```
//
// For a complete example, see `examples/audio_streaming/`.
class AudioReader {
 public:
  // Constructor.
  //
  // Activates the microphone by calling `Enable()` on the given `AudioDriver`.
  // Although the mic is then active, you must call `FillBuffer()` to capture
  // audio into the buffer provided by `Buffer()`.
  //
  // @param driver An audio driver to manage the microphone.
  // @param config A configuration for audio samples.
  AudioReader(AudioDriver* driver, const AudioDriverConfig& config);

  // Destructor.
  // Calls `Disable()` on the `AudioDriver` given to the constructor.
  ~AudioReader();

  // Gets the audio buffer that's populated with samples when you call
  // `FillBuffer()`.
  //
  // @return The buffer where audio samples are or will be stored.
  const std::vector<int32_t>& Buffer() const { return buffer_; }

  // Fills the audio buffer (provided by `Buffer()`) with audio samples from
  // the microphone.
  //
  // This copies audio samples from the internal ring buffer into the
  // buffer provided by `Buffer()` so you can safely process them. This will
  // fetch as many samples as possible, and if you fail to call it fast enough,
  // the internal ring buffer will overflow and increment `OverflowCount()`.
  //
  // The samples match the sample rate and size you specify with
  // `AudioDriverConfig` and pass to the `AudioReader` constructor.
  //
  // @return The number of samples written to the buffer. You'll need this
  // number so you can read the correct amount from the buffer.
  size_t FillBuffer();

  // Discards microphone samples.
  //
  // You should call this before you begin collecting samples in order to avoid
  // audio distortion that may occur when the microphone first starts.
  //
  // @param min_count  Minimum number of samples to drop.
  // @return Number of samples dropped.
  int Drop(int min_count) {
    int count = 0;
    while (count < min_count) count += FillBuffer();
    return count;
  }

  // Gets the number of times that samples from the mic were lost, because you
  // did not read samples fast enough with `FillBuffer()`.
  //
  // @return The number of times that `FillBuffer()` did not receive the length
  // of samples requested (the ring buffer overflowed and samples were lost).
  int OverflowCount() const { return overflow_count_; }

  // Gets the number of times the buffer was not filled when reading from
  // the internal ring buffer.
  //
  // @return The number of times that `FillBuffer()` was called but the buffer
  // received less than `AudioDriverConfig::dma_buffer_size_samples()`.
  int UnderflowCount() const { return underflow_count_; }

 private:
  static void Callback(void* ctx, const int32_t* buf, size_t size);

  AudioDriver* driver_;

  int dma_buffer_size_ms_;
  std::vector<int32_t> buffer_;
  FreeRTOSStreamBuffer<int32_t> ring_buffer_;

  volatile int overflow_count_ = 0;
  volatile int underflow_count_ = 0;
};

// Provides a mechanism for one or more clients to continuously receive audio
// samples from the on-board microphone with a callback function.
//
// This creates a separate FreeRTOS task that's dedicated to fetching
// audio samples from the microphone and passing reference to those audio
// samples to one or more callbacks that you specify with `AddCallback()`.
// `AudioService` copies audio samples from the `AudioDriver` stream buffer into
// its own buffer (actually managed by an internal `AudioReader`) and then sends
// a reference to this buffer to each callback.
//
// If you don't want to immediately process the audio samples inside your
// callback, you can copy the audio samples with `LatestSamples` and then
// another task outside the callback can read the audio from `LatestSamples`.
//
// The microphone remains powered as long as there is at least one callback
// for an `AudioService` client. Otherwise, the microphone is powered off as
// soon as the `AudioService` is destroyed or all callbacks are removed with
// `RemoveCallback()`.
//
// For example, the basic setup for `AudioService` looks like this:
//
// ```
// namespace {
// AudioDriverBuffers</*NumDmaBuffers=*/4, /*DmaBufferSize=*/6 * 1024>
//     g_audio_buffers;
// AudioDriver g_audio_driver(g_audio_buffers);
// }  // namespace
//
// const AudioDriverConfig config{audio::SampleRate::k16000_Hz,
//                                /*num_dma_buffers=*/4,
//                                /*dma_buffer_size_ms=*/30};
// AudioService service(&g_audio_driver, config);
//
// auto id = service.AddCallback(...);
// service.RemoveCallback(id);
// ```
//
// For a complete example, see `examples/yamnet/`.
class AudioService {
 public:
  // The function type that receives new audio samples as a callback,
  // which must be given to `AddCallback()`.
  //
  // @param ctx Extra parameters, defined with `AddCallback()`.
  // @param samples A pointer to the buffer.
  // @param num_samples The number of audio samples in the buffer.
  // @return True if the callback should be continued to be called,
  // false otherwise.
  using Callback = bool (*)(void* ctx, const int32_t* samples,
                            size_t num_samples);

  // Constructor.
  //
  // @param driver An audio driver to manage the microphone.
  // @param config A configuration for audio samples.
  // @param task_priority Priority for internal FreeRTOS task that
  // dispatches audio samples to registered callbacks.
  // @param drop_first_samples_ms Amount, in milliseconds,
  // of audio to drop at the start of recording.
  AudioService(AudioDriver* driver, const AudioDriverConfig& config,
               int task_priority, int drop_first_samples_ms);
  //@cond
  AudioService(const AudioService&) = delete;
  AudioService& operator=(const AudioService&) = delete;
  ~AudioService();
  //@endcond

  // Adds a callback function to receive audio samples.
  //
  // You can add as many callbacks as you want. Each one is identified by
  // a unique id, which you must use if you want to remove the callback with
  // `RemoveCallback()`.
  //
  // @param ctx Extra parameters to pass through to the callback function.
  // @param fn The function to receive audio samples.
  // @return A unique id for the callback function.
  int AddCallback(void* ctx, Callback fn);

  // Removes a callback function.
  //
  // @param id The id of the callback function to remove.
  // @return True if successfully removed, false otherwise.
  bool RemoveCallback(int id);

  // Gets the audio driver configuration.
  //
  // @return The audio driver configuration.
  const AudioDriverConfig& Config() const { return config_; }

 private:
  AudioDriver* driver_;
  AudioDriverConfig config_;
  int drop_first_samples_;
  TaskHandle_t task_;
  QueueHandle_t queue_;

  static void StaticRun(void* param);
  void Run() const;
};

// Provides a structure in which you can copy incoming audio samples and
// read them later. This is designed for use with `AudioService` so that
// your callback function can continuously receive new audio samples and copy
// them into a `LatestSamples` object. This allows another task in your program
// to read the copied samples instead of trying to process the samples as
// they arrive in the callback.
//
// Here's an example that saves the latest 1000 ms of audio samples from
// an `AudioService` callback into `LatestSamples`:
//
// ```
// AudioService* service = ...
//
// LatestSamples latest(audio::MsToSamples(service->sample_rate(), 1000));
// service->AddCallback(
//     &latest, +[](void* ctx, const int32_t* samples, size_t num_samples) {
//         static_cast<LatestSamples*>(ctx)->Append(samples, num_samples);
//         return true;
//     });
// ```
//
// Then you can directly read the latest `num_samples` saved in `LatestSamples`
// and apply a function to them by calling `AccessLatestSamples()` (samples
// received by the function start at `start_index`):
//
// ```
// latest.AccessLatestSamples([](const std::vector<int32_t>& samples,
//                               size_t start_index) {
//     1st: [samples.begin() + start_index, samples.end())
//     2nd: [samples.begin(),               samples.begin() + start_index)
// });
// ```
//
// Or you can get a copy of the latest samples by calling `CopyLatestSamples()`:
//
// ```
// auto last_second = latest.CopyLatestSamples();
// ```
//
// For a complete example, see `examples/yamnet/`.
class LatestSamples {
 public:
  // Constructor.
  //
  // @param num_samples Fixed number of samples that can be saved.
  explicit LatestSamples(size_t num_samples);
  // @cond
  LatestSamples(const LatestSamples&) = delete;
  LatestSamples& operator=(const LatestSamples&) = delete;
  ~LatestSamples();
  // @endcond

  // Gets the number of samples currently saved.
  //
  // @return The number of available samples.
  size_t NumSamples() const { return samples_.size(); };

  // Adds new audio samples to the collection.
  //
  // New samples are appended to the collection at the index
  // position where this function left off after the
  // previous append.
  //
  // You can read these samples without a copy using
  // 'AccessLatestSamples()'. Or get them with a copy using
  // `CopyLatestSamples()`.
  //
  // @param samples A pointer to the buffer position from which you want to
  // begin adding samples.
  // @param num_samples The number of audio samples to add from the buffer.
  void Append(const int32_t* samples, size_t num_samples) {
    MutexLock lock(mutex_);
    for (size_t i = 0; i < num_samples; ++i)
      samples_[(pos_ + i) % samples_.size()] = samples[i];
    pos_ = (pos_ + num_samples) % samples_.size();
  }

  // Gets the latest samples without a copy and applies a function to them.
  //
  // @param f A function to apply to samples. The function receives a reference
  // to the samples as an `int32_t` array and the start index as `size_t`.
  // See the example above, in the `LatestSamples` introduction.
  template <typename F>
  void AccessLatestSamples(F f) const {
    MutexLock lock(mutex_);
    f(samples_, pos_);
  }

  // Gets a copy of the latest samples.
  //
  // This ensures that the samples copied out are actually in chronological
  // order, rather than being a raw copy of the internal array (which can have
  // newer samples at the beginning of the array due to the index position
  // wrapping around after multiple calls to `Append()`).
  //
  // @return A chronological copy of the latest samples.
  std::vector<int32_t> CopyLatestSamples() const {
    MutexLock lock(mutex_);
    std::vector<int32_t> copy(samples_);
    std::rotate(std::begin(copy), std::begin(copy) + pos_, std::end(copy));
    return copy;
  }

 private:
  SemaphoreHandle_t mutex_;
  size_t pos_ = 0;                // protected by mutex_;
  std::vector<int32_t> samples_;  // protected by mutex_;
};

}  // namespace coralmicro

#endif  // LIBS_AUDIO_AUDIO_SERVICE_H_
