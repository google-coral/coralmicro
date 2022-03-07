#include <errno.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "libs/tasks/AudioTask/audio_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

namespace {

constexpr int kPort = 33000;

enum class Status { kOk, kEof, kError };

Status ReadBytes(int fd, void* bytes, size_t size) {
    assert(fd >= 0);
    assert(bytes);

    char* buf = reinterpret_cast<char*>(bytes);
    while (size != 0) {
        auto ret = ::read(fd, buf, size);
        if (ret == 0) return Status::kEof;
        if (ret == -1) {
            if (errno == EINTR) continue;
            return Status::kError;
        }
        size -= ret;
        buf += ret;
    }
    return Status::kOk;
}

template <typename T>
Status ReadArray(int fd, T* array, size_t array_size) {
    return ReadBytes(fd, array, array_size * sizeof(T));
}

Status WriteBytes(int fd, const void* bytes, size_t size,
                  size_t chunk_size = 1024) {
    assert(fd >= 0);
    assert(bytes);

    const char* buf = reinterpret_cast<const char*>(bytes);
    while (size != 0) {
        auto len = std::min(size, chunk_size);
        auto ret = ::write(fd, buf, len);
        if (ret == -1) {
            if (errno == EINTR) continue;
            return Status::kError;
        }
        size -= len;
        buf += len;
    }

    return Status::kOk;
}

template <typename T>
Status WriteArray(int fd, const T* array, size_t array_size) {
    return WriteBytes(fd, array, array_size * sizeof(T));
}

bool VerifySampleRate(int sample_rate_hz,
                      valiant::audio::SampleRate* sample_rate) {
    switch (sample_rate_hz) {
        case 16000:
            *sample_rate = valiant::audio::SampleRate::k16000_Hz;
            return true;
        case 48000:
            *sample_rate = valiant::audio::SampleRate::k48000_Hz;
            return true;
    }
    return false;
}

template <typename T>
class xStreamBuffer {
   public:
    xStreamBuffer() : handle_(nullptr) {}

    ~xStreamBuffer() {
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
    AudioReader(valiant::audio::SampleRate sample_rate, int dma_buffer_size_ms,
                int num_dma_buffers)
        : dma_buffer_size_ms_(dma_buffer_size_ms) {
        const int samples_per_ms = static_cast<int>(sample_rate) / 1000;
        const int samples_per_dma_buffer = dma_buffer_size_ms * samples_per_ms;

        buffer_.resize(samples_per_dma_buffer);
        dma_buffer_.resize(num_dma_buffers * samples_per_dma_buffer);

        ring_buffer_.Create(
            /*xBufferSize=*/samples_per_dma_buffer * num_dma_buffers,
            /*xTriggerLevel=*/samples_per_dma_buffer);
        assert(ring_buffer_.Ok());

        std::vector<int32_t*> dma_buffers;
        for (int i = 0; i < num_dma_buffers; ++i)
            dma_buffers.push_back(dma_buffer_.data() +
                                  i * samples_per_dma_buffer);

        valiant::AudioTask::GetSingleton()->SetPower(true);
        valiant::AudioTask::GetSingleton()->Enable(
            sample_rate, dma_buffers, samples_per_dma_buffer, this, Callback);
    }

    ~AudioReader() {
        valiant::AudioTask::GetSingleton()->Disable();
        valiant::AudioTask::GetSingleton()->SetPower(false);
    }

    const std::vector<int32_t>& Buffer() const { return buffer_; }

    size_t FillBuffer() {
        auto received_size =
            ring_buffer_.Receive(buffer_.data(), buffer_.size(),
                                 pdMS_TO_TICKS(2 * dma_buffer_size_ms_));
        if (received_size != buffer_.size()) ++underflow_count_;
        return received_size;
    }

    int OverflowCount() const { return overflow_count_; }
    int UnderflowCount() const { return underflow_count_; }

   private:
    static void Callback(void* param, const int32_t* buf, size_t size) {
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        auto* self = reinterpret_cast<AudioReader*>(param);
        auto sent_size = self->ring_buffer_.SendFromISR(
            buf, size, &xHigherPriorityTaskWoken);
        if (size != sent_size) ++self->overflow_count_;
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    int dma_buffer_size_ms_;
    std::vector<int32_t> buffer_;
    std::vector<int32_t> dma_buffer_;
    xStreamBuffer<int32_t> ring_buffer_;

    volatile int overflow_count_ = 0;
    volatile int underflow_count_ = 0;
};

void ProcessClient(int client_socket) {
    int32_t params[3];
    if (ReadArray(client_socket, params, 3) != Status::kOk) {
        printf("ERROR: Cannot read params from client socket\n\r");
        return;
    }

    const int sample_rate_hz = params[0];
    const int dma_buffer_size_ms = params[1];
    const int num_dma_buffers = params[2];

    valiant::audio::SampleRate sample_rate;
    if (!VerifySampleRate(sample_rate_hz, &sample_rate)) {
        printf("ERROR: Invalid sample rate: %d\n\r", sample_rate_hz);
        return;
    }

    printf("Format:\n\r");
    printf("  Sample rate (Hz): %d\n\r", sample_rate_hz);
    printf("  DMA buffer size (ms): %d\n\r", dma_buffer_size_ms);
    printf("  # of DMA buffers: %d\n\r", num_dma_buffers);

    AudioReader reader(sample_rate, dma_buffer_size_ms, num_dma_buffers);

    int total_bytes = 0;
    auto& buffer = reader.Buffer();
    while (true) {
        auto size = reader.FillBuffer();
        if (WriteArray(client_socket, buffer.data(), size) != Status::kOk)
            break;
        total_bytes += size * sizeof(int32_t);
    }

    printf("Statistics:\n\r");
    printf("  Bytes sent: %d\n\r", total_bytes);
    printf("  Ring buffer overflows: %d\n\r", reader.OverflowCount());
    printf("  Ring buffer underflows: %d\n\r", reader.UnderflowCount());
}

void run_server() {
    const int server_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        printf("ERROR: Cannot create server socket\n\r");
        return;
    }

    struct sockaddr_in bind_address;
    bind_address.sin_family = AF_INET;
    bind_address.sin_port = PP_HTONS(kPort);
    bind_address.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    auto ret =
        ::bind(server_socket, reinterpret_cast<struct sockaddr*>(&bind_address),
               sizeof(bind_address));
    if (ret == -1) {
        printf("ERROR: Cannot bind server socket\n\r");
        return;
    }

    ret = ::listen(server_socket, 1);
    if (ret == -1) {
        printf("ERROR: Cannot listen server socket\n\r");
        return;
    }

    while (true) {
        printf("=> Waiting for the client...\n\r");
        const int client_socket = ::accept(server_socket, nullptr, nullptr);
        printf("=> Client connected.\n\r");
        ProcessClient(client_socket);
        ::closesocket(client_socket);
        printf("=> Client disconnected.\n\r");
    }
}

}  // namespace

extern "C" void app_main(void* param) {
    run_server();
    vTaskSuspend(NULL);
}
