#include "libs/YAMNet/yamnet.h"

#include "libs/audio/audio_service.h"
#include "libs/base/filesystem.h"

namespace {
constexpr int kNumDmaBuffers = 2;
constexpr int kDmaBufferSizeMs = 50;
constexpr int kDmaBufferSize =
    kNumDmaBuffers * coral::micro::yamnet::kSampleRateMs * kDmaBufferSizeMs;
constexpr int kAudioServicePriority = 4;
constexpr int kDropFirstSamplesMs = 150;

std::vector<uint8_t> yamnet_test_input_bin;
std::unique_ptr<coral::micro::LatestSamples> audio_latest = nullptr;

coral::micro::AudioDriverBuffers<kNumDmaBuffers, kDmaBufferSize> audio_buffers;
coral::micro::AudioDriver audio_driver(audio_buffers);

constexpr int kAudioBufferSizeMs = coral::micro::yamnet::kDurationMs;
constexpr int kAudioBufferSize =
    kAudioBufferSizeMs * coral::micro::yamnet::kSampleRateMs;

}  // namespace

extern "C" [[noreturn]] void app_main(void* param) {
    if (!coral::micro::yamnet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }

    if (!coral::micro::filesystem::ReadFile("/models/yamnet_test_audio.bin",
                                            &yamnet_test_input_bin)) {
        printf("Failed to load test input!\r\n");
        vTaskSuspend(nullptr);
    }

    if (yamnet_test_input_bin.size() !=
        coral::micro::yamnet::kAudioSize * sizeof(int16_t)) {
        printf("Input audio size doesn't match expected\r\n");
        vTaskSuspend(nullptr);
    }

    std::shared_ptr<int16_t[]> audio_data = coral::micro::yamnet::audio_input();
    std::memcpy(audio_data.get(), yamnet_test_input_bin.data(),
                yamnet_test_input_bin.size());
    coral::micro::yamnet::loop();
    printf("YAMNet Setup Complete\r\n\n");

    // Setup audio
    coral::micro::AudioDriverConfig audio_config{
        coral::micro::AudioSampleRate::k16000_Hz, kNumDmaBuffers,
        kDmaBufferSizeMs};
    coral::micro::AudioService audio_service(&audio_driver, audio_config,
                                             kAudioServicePriority,
                                             kDropFirstSamplesMs);

    audio_latest = std::make_unique<coral::micro::LatestSamples>(
        coral::micro::MsToSamples(coral::micro::AudioSampleRate::k16000_Hz,
                                  coral::micro::yamnet::kDurationMs));
    audio_service.AddCallback(
        audio_latest.get(),
        +[](void* ctx, const int32_t* samples, size_t num_samples) {
            static_cast<coral::micro::LatestSamples*>(ctx)->Append(samples,
                                                                   num_samples);
            return true;
        });

    // Delay for the first buffers to fill.
    vTaskDelay(pdMS_TO_TICKS(coral::micro::yamnet::kDurationMs));

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
        coral::micro::yamnet::loop();
    }
    vTaskSuspend(nullptr);
}
