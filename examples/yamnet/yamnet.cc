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

#include "libs/yamnet/yamnet.h"

#include "libs/audio/audio_service.h"
#include "libs/base/filesystem.h"

namespace {
constexpr int kNumDmaBuffers = 2;
constexpr int kDmaBufferSizeMs = 50;
constexpr int kDmaBufferSize =
    kNumDmaBuffers * coralmicro::yamnet::kSampleRateMs * kDmaBufferSizeMs;
constexpr int kAudioServicePriority = 4;
constexpr int kDropFirstSamplesMs = 150;

std::vector<uint8_t> yamnet_test_input_bin;
std::unique_ptr<coralmicro::LatestSamples> audio_latest = nullptr;

coralmicro::AudioDriverBuffers<kNumDmaBuffers, kDmaBufferSize> audio_buffers;
coralmicro::AudioDriver audio_driver(audio_buffers);

constexpr int kAudioBufferSizeMs = coralmicro::yamnet::kDurationMs;
constexpr int kAudioBufferSize =
    kAudioBufferSizeMs * coralmicro::yamnet::kSampleRateMs;

}  // namespace

extern "C" [[noreturn]] void app_main(void* param) {
    if (!coralmicro::yamnet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }

    if (!coralmicro::filesystem::ReadFile("/models/yamnet_test_audio.bin",
                                            &yamnet_test_input_bin)) {
        printf("Failed to load test input!\r\n");
        vTaskSuspend(nullptr);
    }

    if (yamnet_test_input_bin.size() !=
        coralmicro::yamnet::kAudioSize * sizeof(int16_t)) {
        printf("Input audio size doesn't match expected\r\n");
        vTaskSuspend(nullptr);
    }

    std::shared_ptr<int16_t[]> audio_data = coralmicro::yamnet::audio_input();
    std::memcpy(audio_data.get(), yamnet_test_input_bin.data(),
                yamnet_test_input_bin.size());
    coralmicro::yamnet::loop();
    printf("YAMNet Setup Complete\r\n\n");

    // Setup audio
    coralmicro::AudioDriverConfig audio_config{
        coralmicro::AudioSampleRate::k16000_Hz, kNumDmaBuffers,
        kDmaBufferSizeMs};
    coralmicro::AudioService audio_service(&audio_driver, audio_config,
                                             kAudioServicePriority,
                                             kDropFirstSamplesMs);

    audio_latest = std::make_unique<coralmicro::LatestSamples>(
        coralmicro::MsToSamples(coralmicro::AudioSampleRate::k16000_Hz,
                                  coralmicro::yamnet::kDurationMs));
    audio_service.AddCallback(
        audio_latest.get(),
        +[](void* ctx, const int32_t* samples, size_t num_samples) {
            static_cast<coralmicro::LatestSamples*>(ctx)->Append(samples,
                                                                   num_samples);
            return true;
        });

    // Delay for the first buffers to fill.
    vTaskDelay(pdMS_TO_TICKS(coralmicro::yamnet::kDurationMs));

    while (true) {
        audio_latest->AccessLatestSamples(
            [&audio_data](const std::vector<int32_t>& samples,
                          size_t start_index) {
                size_t i, j = 0;
                // Starting with start_index, grab until the end of the buffer.
                for (i = 0; i < samples.size() - start_index; ++i) {
                    audio_data[i] = samples[i + start_index] >> 16;
                }
                // Now fill the rest of the data with the beginning of the
                // buffer.
                for (j = 0; j < samples.size() - i; ++j) {
                    audio_data[i + j] = samples[j] >> 16;
                }
            });
        coralmicro::yamnet::loop();
#ifndef YAMNET_CPU
        // Delay 975 ms to rate limit the TPU version.
        vTaskDelay(pdMS_TO_TICKS(coralmicro::yamnet::kDurationMs));
#endif
    }
    vTaskSuspend(nullptr);
}
