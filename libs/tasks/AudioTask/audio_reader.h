#ifndef _LIBS_TASKS_AUDIO_READER_
#define _LIBS_TASKS_AUDIO_READER_

#include <algorithm>
#include <cstdint>
#include <vector>

#include "libs/base/mutex.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"

namespace coral::micro {

template <typename T>
class FreeRTOSStreamBuffer {
   public:
    FreeRTOSStreamBuffer() : handle_(nullptr) {}

    ~FreeRTOSStreamBuffer() {
        if (handle_) vStreamBufferDelete(handle_);
    }

    bool Ok() const { return handle_ != nullptr; }

    void Create(size_t xBufferSize, size_t xTriggerLevel) {
        handle_ = xStreamBufferCreate(xBufferSize * sizeof(T),
                                      xTriggerLevel * sizeof(T));
    }

    size_t Send(const T* pvTxData, size_t xDataLength,
                TickType_t xTicksToWait) {
        return xStreamBufferSend(handle_, pvTxData, xDataLength * sizeof(T),
                                 xTicksToWait) /
               sizeof(T);
    }

    size_t SendFromISR(const T* pvTxData, size_t xDataLength,
                       BaseType_t* pxHigherPriorityTaskWoken) {
        return xStreamBufferSendFromISR(handle_, pvTxData,
                                        xDataLength * sizeof(T),
                                        pxHigherPriorityTaskWoken) /
               sizeof(T);
    }

    size_t Receive(T* pvRxData, size_t xBufferLength, TickType_t xTicksToWait) {
        return xStreamBufferReceive(handle_, pvRxData,
                                    xBufferLength * sizeof(T), xTicksToWait) /
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

// Class to read audio samples from the microphone in real time.
//
// Example:
//
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
//
class AudioReader {
   public:
    AudioReader(AudioDriver* driver, const AudioDriverConfig& config);
    ~AudioReader();

    const std::vector<int32_t>& Buffer() const { return buffer_; }

    size_t FillBuffer();

    int Drop(int min_count) {
        int count = 0;
        while (count < min_count) count += FillBuffer();
        return count;
    }

    int OverflowCount() const { return overflow_count_; }
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
// the `AudioServiceCallback` function to continuously receive new microphone
// samples. Internally microphone is only enabled when there is at least one
// client, otherwise microphone is completely disabled (no power is consumed).
//
// Example:
//
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
//
class AudioService {
   public:
    using Callback = bool (*)(void* ctx, const int32_t* samples,
                              size_t num_samples);

    AudioService(AudioDriver* driver, const AudioDriverConfig& config,
                 int task_priority, int drop_first_samples_ms);
    AudioService(const AudioService&) = delete;
    AudioService& operator=(const AudioService&) = delete;
    ~AudioService();

    int AddCallback(void* ctx, Callback fn);
    bool RemoveCallback(int id);

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
//     AudioService* service = ...
//
//     LatestSamples latest(audio::MsToSamples(service->sample_rate(), 1000));
//     service->AddCallback(
//         &latest, +[](void* ctx, const int32_t* samples, size_t num_samples) {
//             static_cast<LatestSamples*>(ctx)->Append(samples, num_samples);
//             return true;
//         });
//
// Call `AccessLatestSamples()` to access the latest `num_samples` without a
// copy. Samples start at `start_index`:
//
//     latest.AccessLatestSamples([](const std::vector<int32_t>& samples,
//                                   size_t start_index) {
//         1st: [samples.begin() + start_index, samples.end())
//         2nd: [samples.begin(),               samples.begin() + start_index)
//     });
//
// Call `CopyLatestSamples()` to get a copy of latest `num_samples`:
//
//     auto last_second = latest.CopyLatestSamples();
//
class LatestSamples {
   public:
    LatestSamples(size_t num_samples);
    LatestSamples(const LatestSamples&) = delete;
    LatestSamples& operator=(const LatestSamples&) = delete;
    ~LatestSamples();

    size_t NumSamples() const { return samples_.size(); };

    void Append(const int32_t* samples, size_t num_samples) {
        MutexLock lock(mutex_);
        for (size_t i = 0; i < num_samples; ++i)
            samples_[(pos_ + i) % samples_.size()] = samples[i];
        pos_ = (pos_ + num_samples) % samples_.size();
    }

    template <typename F>
    void AccessLatestSamples(F f) const {
        MutexLock lock(mutex_);
        f(samples_, pos_);
    }

    std::vector<int32_t> CopyLatestSamples() const {
        MutexLock lock(mutex_);
        std::vector<int32_t> copy(samples_);
        std::rotate(std::begin(copy), std::begin(copy) + pos_, std::end(copy));
        return copy;
    }

   private:
    SemaphoreHandle_t mutex_;
    size_t pos_ = 0;  // protected by mutex_;
    std::vector<int32_t> samples_;  // protected by mutex_;
};

}  // namespace coral::micro

#endif  // _LIBS_TASKS_AUDIO_READER_
