#include <cstdint>
#include <vector>

#include "libs/tasks/AudioTask/audio_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
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

}  // namespace valiant
