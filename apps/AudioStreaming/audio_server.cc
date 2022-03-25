#include <errno.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "libs/tasks/AudioTask/audio_reader.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

namespace {

constexpr int kPort = 33000;

enum class Status { kOk, kEof, kError };

constexpr int kNumSampleFormats = 2;
constexpr const char* kSampleFormatNames[] = {"S16_LE", "S32_LE"};
enum SampleFormat {
    kS16LE = 0,
    kS32LE = 1,
};

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

void ProcessClient(int client_socket) {
    int32_t params[4];
    if (ReadArray(client_socket, params, 4) != Status::kOk) {
        printf("ERROR: Cannot read params from client socket\n\r");
        return;
    }

    const int sample_rate_hz = params[0];
    const int sample_format = params[1];
    const int dma_buffer_size_ms = params[2];
    const int num_dma_buffers = params[3];

    valiant::audio::SampleRate sample_rate;
    if (!VerifySampleRate(sample_rate_hz, &sample_rate)) {
        printf("ERROR: Invalid sample rate: %d\n\r", sample_rate_hz);
        return;
    }

    if (sample_format < 0 || sample_format >= kNumSampleFormats) {
        printf("ERROR: Invalid sample format: %d\n\r", sample_format);
        return;
    }

    printf("Format:\n\r");
    printf("  Sample rate (Hz): %d\n\r", sample_rate_hz);
    printf("  Sample format: %s\n\r", kSampleFormatNames[sample_format]);
    printf("  DMA buffer size (ms): %d\n\r", dma_buffer_size_ms);
    printf("  DMA buffer count: %d\n\r", num_dma_buffers);

    valiant::AudioReader reader(sample_rate, dma_buffer_size_ms,
                                num_dma_buffers);

    int total_bytes = 0;
    auto& buffer32 = reader.Buffer();

    if (sample_format == kS32LE) {
        while (true) {
            auto size = reader.FillBuffer();
            if (WriteArray(client_socket, buffer32.data(), size) != Status::kOk)
                break;
            total_bytes += size * sizeof(int32_t);
        }
    } else {
        std::vector<int16_t> buffer16(buffer32.size());
        while (true) {
            auto size = reader.FillBuffer();
            for (size_t i = 0; i < size; ++i) buffer16[i] = buffer32[i] >> 16;
            if (WriteArray(client_socket, buffer16.data(), size) != Status::kOk)
                break;
            total_bytes += size * sizeof(int16_t);
        }
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
