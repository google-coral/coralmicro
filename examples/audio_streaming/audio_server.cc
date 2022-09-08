// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <cstdio>

#include "libs/audio/audio_service.h"
#include "libs/base/led.h"
#include "libs/base/network.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

// Captures audio from the Dev Board Micro on-board microphone
// and makes the audio stream available from a local RPC server that you can
// connect to with a Python client.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e audio_server
//
// NOTE: The Python client app works with Windows and Linux only.
//
// Then receive the audio stream over USB:
//    python3 -m pip install -r examples/audio_server/requirements.txt
//    python3 examples/audio_server/audio_client.py

namespace coralmicro {
namespace {
AudioDriverBuffers</*NumDmaBuffers*/ 16, /*CombinedDmaBufferSize=*/28 * 1024>
    g_audio_buffers;

constexpr int kPort = 33000;
constexpr int kNumSampleFormats = 2;
constexpr const char* kSampleFormatNames[] = {"S16_LE", "S32_LE"};
enum SampleFormat {
  kS16LE = 0,
  kS32LE = 1,
};

void ProcessClient(int client_socket) {
  int32_t params[5];
  if (ReadArray(client_socket, params, std::size(params)) != IOStatus::kOk) {
    printf("ERROR: Cannot read params from client socket\r\n");
    return;
  }

  const int sample_rate_hz = params[0];
  const int sample_format = params[1];
  const int dma_buffer_size_ms = params[2];
  const int num_dma_buffers = params[3];
  const int drop_first_samples_ms = params[4];

  auto sample_rate = CheckSampleRate(sample_rate_hz);
  if (!sample_rate.has_value()) {
    printf("ERROR: Invalid sample rate (Hz): %d\r\n", sample_rate_hz);
    return;
  }

  if (sample_format < 0 || sample_format >= kNumSampleFormats) {
    printf("ERROR: Invalid sample format: %d\r\n", sample_format);
    return;
  }

  if (dma_buffer_size_ms <= 0) {
    printf("ERROR: Invalid DMA buffer size (ms): %d\r\n", dma_buffer_size_ms);
    return;
  }

  if (num_dma_buffers <= 0) {
    printf("ERROR: Invalid number of DMA buffers: %d\r\n", num_dma_buffers);
    return;
  }

  AudioDriver driver(g_audio_buffers);
  const AudioDriverConfig config{*sample_rate,
                                 static_cast<size_t>(num_dma_buffers),
                                 static_cast<size_t>(dma_buffer_size_ms)};
  if (!g_audio_buffers.CanHandle(config)) {
    printf("ERROR: Not enough static memory for DMA buffers\r\n");
    return;
  }

  printf("Format:\r\n");
  printf("  Sample rate (Hz): %d\r\n", sample_rate_hz);
  printf("  Sample format: %s\r\n", kSampleFormatNames[sample_format]);
  printf("  DMA buffer size (ms): %d\r\n", config.dma_buffer_size_ms);
  printf("  DMA buffer size (samples): %d\r\n",
         config.dma_buffer_size_samples());
  printf("  DMA buffer count: %d (max %d)\r\n", config.num_dma_buffers,
         g_audio_buffers.kNumDmaBuffers);
  printf("  Combined DMA buffer size (samples): %d (max %d)\r\n",
         config.num_dma_buffers * config.dma_buffer_size_samples(),
         g_audio_buffers.kCombinedDmaBufferSize);
  printf("Sending audio samples...\r\n");

  AudioReader reader(&driver, config);
  const auto& buffer32 = reader.Buffer();

  const int num_dropped_samples =
      reader.Drop(MsToSamples(*sample_rate, drop_first_samples_ms));

  int total_bytes = 0;
  if (sample_format == kS32LE) {
    while (true) {
      auto size = reader.FillBuffer();
      if (WriteArray(client_socket, buffer32.data(), size) != IOStatus::kOk)
        break;
      total_bytes += size * sizeof(int32_t);
    }
  } else {
    std::vector<int16_t> buffer16(buffer32.size());
    while (true) {
      auto size = reader.FillBuffer();
      for (size_t i = 0; i < size; ++i) buffer16[i] = buffer32[i] >> 16;
      if (WriteArray(client_socket, buffer16.data(), size) != IOStatus::kOk)
        break;
      total_bytes += size * sizeof(int16_t);
    }
  }

  printf("Bytes sent: %d\r\n", total_bytes);
  printf("Ring buffer overflows: %d\r\n", reader.OverflowCount());
  printf("Ring buffer underflows: %d\r\n", reader.UnderflowCount());
  printf("Dropped first samples: %d\r\n", num_dropped_samples);
  printf("Done.\r\n\r\n");
}

[[noreturn]] void Main() {
  printf("Audio Streaming Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  const int server_socket = SocketServer(kPort, 5);
  if (server_socket == -1) {
    printf("ERROR: Cannot start server.\r\n");
    vTaskSuspend(nullptr);
  }

  while (true) {
    printf("INFO: Waiting for the client...\r\n");
    const int client_socket = ::accept(server_socket, nullptr, nullptr);
    if (client_socket == -1) {
      printf("ERROR: Cannot connect client.\r\n");
      continue;
    }

    printf("INFO: Client #%d connected.\r\n", client_socket);
    ProcessClient(client_socket);
    ::closesocket(client_socket);
    printf("INFO: Client #%d disconnected.\r\n", client_socket);
  }
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
