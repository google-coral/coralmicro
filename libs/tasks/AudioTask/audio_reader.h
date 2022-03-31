#include <cstdint>
#include <vector>

#include "libs/base/mutex.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"

namespace valiant {

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
// AudioReader reader(audio::SampleRate::k16000_Hz,
//                    /*dma_buffer_size_ms=*/100,
//                    /*num_dma_buffers=*/10);
//
// auto& buffer = reader.Buffer();
// while (true) {
//     auto size = reader.FillBuffer();
//     ProcessBuffer(buffer.data(), size);
// }
//
class AudioReader {
   public:
    AudioReader(audio::SampleRate sample_rate, int dma_buffer_size_ms,
                int num_dma_buffers);
    ~AudioReader();

    const std::vector<int32_t>& Buffer() const { return buffer_; }

    size_t FillBuffer();

    int OverflowCount() const { return overflow_count_; }
    int UnderflowCount() const { return underflow_count_; }

   private:
    static void Callback(void* param, const int32_t* buf, size_t size);

    int dma_buffer_size_ms_;
    std::vector<int32_t> buffer_;
    std::vector<int32_t> dma_buffer_;
    FreeRTOSStreamBuffer<int32_t> ring_buffer_;

    volatile int overflow_count_ = 0;
    volatile int underflow_count_ = 0;
};

// Class to access fixed number of latest audio samples from the microphone.
// Call `AccessLatestSamples()` at any time to access the latest
// `latest_buffer_size_ms` of audio data. Samples start at `start_index`:
//
//   First part:  [samples.begin() + start_index, samples.end())
//   Second part: [samples.begin(),               samples.begin() + start_index)
//
// Example:
//
// LatestAudioReader reader(audio::SampleRate::k16000_Hz,
//                          /*dma_buffer_size_ms=*/100,
//                          /*num_dma_buffers=*/10,
//                          /*latest_buffer_size_ms=*/150);
//
// reader.AccessLatestSamples(
//    [](const std::vector<int32_t>& samples, size_t start_index) { ... });
//
class LatestAudioReader {
   public:
    LatestAudioReader(audio::SampleRate sample_rate, int dma_buffer_size_ms,
                      int num_dma_buffers, int latest_buffer_size_ms);
    ~LatestAudioReader();

    size_t NumSamples() const { return samples_.size(); }

    template <typename F>
    void AccessLatestSamples(F f) {
        MutexLock lock(mutex_);
        f(samples_, pos_);
    };

   private:
    static void StaticRun(void* param);
    void Run();

    audio::SampleRate sample_rate_;
    int dma_buffer_size_ms_;
    int num_dma_buffers_;

    TaskHandle_t task_;
    SemaphoreHandle_t mutex_;
    std::vector<int32_t> samples_;  // protected by mutex_;
    size_t pos_ = 0;                // protected by mutex_;
    bool done_ = false;             // protected by mutex_;
};

}  // namespace valiant
