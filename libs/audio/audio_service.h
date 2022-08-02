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

// Class to read audio samples from the on-board PDM microphone in real time.
//
// Example:
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
class AudioReader {
 public:
  // Constructor for an AudioReader object.
  // Calls Enable() from driver to start receiving audio.
  //
  // @param driver Pointer to the AudioDriver to be used by the audio reader.
  // @param config Configuration for driver.
  AudioReader(AudioDriver* driver, const AudioDriverConfig& config);
  // Calls Disable() from driver.
  ~AudioReader();

  // Gets the buffer that the AudioService populates with samples.
  //
  // @return Buffer containing samples.
  const std::vector<int32_t>& Buffer() const { return buffer_; }

  // Fills the audio buffer audio data from the microphone of size sample_size
  // defined in config.
  //
  // @return Size of received data.
  size_t FillBuffer();

  // Discards a minimum amount of samples.
  // Drop is used to avoid distortion caused when connecting to audio
  // for the first time.
  //
  // @param min_count  Minimum number of samples to drop.
  // @return Number of samples dropped.
  int Drop(int min_count) {
    int count = 0;
    while (count < min_count) count += FillBuffer();
    return count;
  }
  // Gets the amount of times that the buffer did not receive all the samples
  // that were sent.
  //
  // @return The amount of times that the buffer did not receive all the samples
  // that were sent.
  int OverflowCount() const { return overflow_count_; }
  // Gets the amount of times that the buffer did not store all the samples that
  // were received.
  //
  // @return The amount of times that the buffer did not store all the samples
  // that were received.
  int UnderflowCount() const { return underflow_count_; }

 private:
  static void Callback(void* param, const int32_t* buf, size_t size);

  AudioDriver* driver_;

  int dma_buffer_size_ms_;
  std::vector<int32_t> buffer_;
  FreeRTOSStreamBuffer<int32_t> ring_buffer_;

  volatile int overflow_count_ = 0;
  volatile int underflow_count_ = 0;
};

// Class to get audio samples from the microphone. Each client must provide
// the `AudioService::Callback` function to continuously receive new microphone
// samples. Internally microphone is only enabled when there is at least one
// client, otherwise microphone is completely disabled (no power is consumed).
//
// Example:
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
class AudioService {
 public:
  // Defines the function type that processes
  // new audio samples. To add a Callback use `AddCallback()`.
  // Callback is called automatically when running AudioService by
  // a dedicated FreeRTOS task.
  //
  // @param ctx Extra parameters for the callback function.
  // @param samples Pointer to the audio samples.
  // @param num_samples Number of audio samples.
  // @return True if the callback should be continued to be called,
  // false otherwise.
  using Callback = bool (*)(void* ctx, const int32_t* samples,
                            size_t num_samples);

  // Constructor for an AudioService object.
  //
  // @param driver Pointer to the AudioDriver to be used by the audio reader.
  // @param config Configuration for driver.
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

  // Adds a CallBack function, that defines how to process audio, to a list of
  // callback methods that are continuously called on by a dedicated FreeRTOS
  // task.
  //
  // @param ctx Extra params for fn.
  // @param fn the Callback function to be registered.
  // @return id of the callback function, fn, to be registered.
  // This id is used as the param for RemoveCallBack() to remove
  // the specific callback function.
  int AddCallback(void* ctx, Callback fn);
  // Removes already registered callback.
  //
  // @param id Id of the CallBack function to remove.
  // @return True if successfully removed, false otherwise.
  bool RemoveCallback(int id);

  // Gets the audio driver config.
  //
  // @return the audio driver config.
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

// Class to access fixed number of latest audio samples from the microphone.
//
// Typical setup to access the latest 1000 ms of audio samples:
//
// ```
//     AudioService* service = ...
//
//     LatestSamples latest(audio::MsToSamples(service->sample_rate(), 1000));
//     service->AddCallback(
//         &latest, +[](void* ctx, const int32_t* samples, size_t num_samples) {
//             static_cast<LatestSamples*>(ctx)->Append(samples, num_samples);
//             return true;
//         });
// ```
//
// Call `AccessLatestSamples()` to access the latest `num_samples` without a
// copy. Samples start at `start_index`:
//
// ```
//     latest.AccessLatestSamples([](const std::vector<int32_t>& samples,
//                                   size_t start_index) {
//         1st: [samples.begin() + start_index, samples.end())
//         2nd: [samples.begin(),               samples.begin() + start_index)
//     });
// ```
//
// Call `CopyLatestSamples()` to get a copy of latest `num_samples`:
//
// ```
//     auto last_second = latest.CopyLatestSamples();
// ```
//
class LatestSamples {
 public:
  // Constructor for LatestSamples object.
  //
  // @param num_samples Fixed number of samples to access.
  explicit LatestSamples(size_t num_samples);
  // @cond
  LatestSamples(const LatestSamples&) = delete;
  LatestSamples& operator=(const LatestSamples&) = delete;
  ~LatestSamples();
  // @endcond

  // Gets the size of the samples to be accessed.
  //
  // @return The size of the samples to be accessed.
  size_t NumSamples() const { return samples_.size(); };

  // Saves the latest samples which can be accessed with
  // 'AccessLatestSamples()'.
  //
  // @param samples Pointer to the audio samples.
  // @param num_samples Number of audio samples.
  void Append(const int32_t* samples, size_t num_samples) {
    MutexLock lock(mutex_);
    for (size_t i = 0; i < num_samples; ++i)
      samples_[(pos_ + i) % samples_.size()] = samples[i];
    pos_ = (pos_ + num_samples) % samples_.size();
  }

  // Accesses the latest samples without a copy and applies a function to them.
  //
  // @param f Function to apply to samples.
  // Function params are: std::vector<int32_t> samples (the latest samples),
  // size_t pos (starting index of samples)
  template <typename F>
  void AccessLatestSamples(F f) const {
    MutexLock lock(mutex_);
    f(samples_, pos_);
  }

  // Creates a copy of the latest samples.
  //
  // @return Copy of the latest samples.
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
