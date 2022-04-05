#include "libs/base/filesystem.h"
#include "libs/tasks/AudioTask/audio_reader.h"
#include "libs/YAMNet/yamnet.h"

namespace {
constexpr int kNumDmaBuffers = 2;
constexpr int kDmaBufferSizeMs = 50;
constexpr int kDmaBufferSize = kNumDmaBuffers * coral::micro::yamnet::kSampleRateMs * kDmaBufferSizeMs;
constexpr int kAudioServicePriority = 4;
constexpr int kDropFirstSamplesMs = 150;

std::unique_ptr<coral::micro::LatestSamples> audio_latest = nullptr;

coral::micro::AudioDriverBuffers<kNumDmaBuffers, kDmaBufferSize> audio_buffers;
coral::micro::AudioDriver audio_driver(audio_buffers);

constexpr int kAudioBufferSizeMs = coral::micro::yamnet::kDurationMs;
constexpr int kAudioBufferSize = kAudioBufferSizeMs * coral::micro::yamnet::kSampleRateMs;

} // namespace

extern "C" [[noreturn]] void app_main(void *param) {
    std::shared_ptr<std::vector<coral::micro::tensorflow::Class>> output = nullptr;

    if (!coral::micro::yamnet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }
    coral::micro::yamnet::loop(output);
    printf("Yamnet setup complete\r\n\n"); vTaskDelay(pdMS_TO_TICKS(100));

    // Setup audio
    coral::micro::AudioDriverConfig audio_config{coral::micro::AudioSampleRate::k16000_Hz,
                                      kNumDmaBuffers, kDmaBufferSizeMs};
    coral::micro::AudioService audio_service(&audio_driver, audio_config, kAudioServicePriority, kDropFirstSamplesMs);

    audio_latest = std::make_unique<coral::micro::LatestSamples>(coral::micro::MsToSamples(coral::micro::AudioSampleRate::k16000_Hz, coral::micro::yamnet::kDurationMs));
    audio_service.AddCallback(
        audio_latest.get(), +[](void* ctx, const int32_t* samples, size_t num_samples) {
            static_cast<coral::micro::LatestSamples*>(ctx)->Append(samples, num_samples);
            return true;
        });

    // Delay for the first buffers to fill.
    vTaskDelay(pdMS_TO_TICKS(coral::micro::yamnet::kDurationMs));

    std::shared_ptr<int16_t[]> audio_data = coral::micro::yamnet::audio_input();
    while (true) {
        audio_latest.get()->AccessLatestSamples([&audio_data](const std::vector<int32_t>& samples, size_t start_index) {
            size_t i,j = 0;
            // Starting with start_index, grab until the end of the buffer.
            for (i = 0; i < samples.size() - start_index; ++i) {
                audio_data[i] = samples[i + start_index] >> 16;
            }
            // Now fill the rest of the data with the beginning of the buffer.
            for (j = 0; j < samples.size() - i; ++j) {
                audio_data[i + j] = samples[j] >> 16;
            }
        });
        coral::micro::yamnet::loop(output);
    }
    vTaskSuspend(nullptr);
}
