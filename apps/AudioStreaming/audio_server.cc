#include <errno.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "apps/AudioStreaming/network.h"
#include "libs/tasks/AudioTask/audio_reader.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/stream_buffer.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

namespace valiant {
namespace {
constexpr int kPort = 33000;
constexpr int kNumSampleFormats = 2;
constexpr const char* kSampleFormatNames[] = {"S16_LE", "S32_LE"};
enum SampleFormat {
    kS16LE = 0,
    kS32LE = 1,
};

void ProcessClient(int client_socket) {
    int32_t params[4];
    if (ReadArray(client_socket, params, 4) != IOStatus::kOk) {
        printf("ERROR: Cannot read params from client socket\n\r");
        return;
    }

    const int sample_rate_hz = params[0];
    const int sample_format = params[1];
    const int dma_buffer_size_ms = params[2];
    const int num_dma_buffers = params[3];

    auto sample_rate = CheckSampleRate(sample_rate_hz);
    if (!sample_rate.has_value()) {
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

    AudioReader reader(sample_rate.value(), dma_buffer_size_ms,
                       num_dma_buffers);

    int total_bytes = 0;
    auto& buffer32 = reader.Buffer();

    if (sample_format == kS32LE) {
        while (true) {
            auto size = reader.FillBuffer();
            if (WriteArray(client_socket, buffer32.data(), size) !=
                IOStatus::kOk)
                break;
            total_bytes += size * sizeof(int32_t);
        }
    } else {
        std::vector<int16_t> buffer16(buffer32.size());
        while (true) {
            auto size = reader.FillBuffer();
            for (size_t i = 0; i < size; ++i) buffer16[i] = buffer32[i] >> 16;
            if (WriteArray(client_socket, buffer16.data(), size) !=
                IOStatus::kOk)
                break;
            total_bytes += size * sizeof(int16_t);
        }
    }

    printf("Statistics:\n\r");
    printf("  Bytes sent: %d\n\r", total_bytes);
    printf("  Ring buffer overflows: %d\n\r", reader.OverflowCount());
    printf("  Ring buffer underflows: %d\n\r", reader.UnderflowCount());
}

void RunServer() {
    const int server_socket = SocketServer(kPort, 5);
    if (server_socket == -1) {
        printf("ERROR: Cannot start server.\n\r");
        return;
    }

    while (true) {
        printf("INFO: Waiting for the client...\n\r");
        const int client_socket = ::accept(server_socket, nullptr, nullptr);
        if (client_socket == -1) {
            printf("ERROR: Cannot connect client.\n\r");
            continue;
        }

        printf("INFO: Client #%d connected.\n\r", client_socket);
        ProcessClient(client_socket);
        ::closesocket(client_socket);
        printf("INFO: Client #%d disconnected.\n\r", client_socket);
    }
}
}  // namespace
}  // namespace valiant

extern "C" void app_main(void* param) {
    valiant::RunServer();
    vTaskSuspend(nullptr);
}
