#include "libs/base/tasks.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/audio_provider.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/main_functions.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.h"

#include <cstdio>
#include <vector>
#include <atomic>

namespace {
constexpr int kSamplesPerMs = kAudioSampleFrequency / 1000;

constexpr int kNumDmaBuffers = 10;
constexpr int kDmaBufferSizeMs = 100;
constexpr int kDmaBufferSize = kDmaBufferSizeMs * kSamplesPerMs;
int32_t g_dma_buffers[kNumDmaBuffers][kDmaBufferSize] __attribute__((aligned(16)));

constexpr int kAudioBufferSizeMs = 1000;
constexpr int kAudioBufferSize = kAudioBufferSizeMs * kSamplesPerMs;
int16_t g_audio_buffer[kAudioBufferSize] __attribute__((aligned(16)));
std::atomic<int32_t> g_audio_buffer_end_index = 0;

int16_t g_audio_buffer_out[kMaxAudioSampleSize] __attribute__((aligned(16)));
}  // namespace

// From audio_provider.h
int32_t LatestAudioTimestamp() {
    return g_audio_buffer_end_index / kSamplesPerMs - 50;
}

// From audio_provider.h
TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int start_ms, int duration_ms,
                             int* audio_samples_size, int16_t** audio_samples) {
    int32_t audio_buffer_end_index = g_audio_buffer_end_index;

    auto buffer_end_ms = audio_buffer_end_index / kSamplesPerMs;
    auto buffer_start_ms = buffer_end_ms - kAudioBufferSizeMs;

    if (start_ms < buffer_start_ms) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "start_ms < buffer_start_ms (%d vs %d)", start_ms, buffer_start_ms);
        return kTfLiteError;
    }

    if (start_ms + duration_ms >= buffer_end_ms) {
        TF_LITE_REPORT_ERROR(error_reporter, "start_ms + duration_ms > buffer_end_ms");
        return kTfLiteError;
    }

    int offset = audio_buffer_end_index + (start_ms - buffer_start_ms) * kSamplesPerMs;
    for (int i = 0; i < kMaxAudioSampleSize; ++i)
        g_audio_buffer_out[i] = g_audio_buffer[(offset + i) % kAudioBufferSize];

    *audio_samples = g_audio_buffer_out;
    *audio_samples_size = kMaxAudioSampleSize;
    return kTfLiteOk;
}

extern "C" void app_main(void *param) {
    printf("Micro speech\r\n");

    // Setup audio
    std::vector<int32_t*> dma_buffers;
    for (int i = 0; i < kNumDmaBuffers; ++i)
        dma_buffers.push_back(g_dma_buffers[i]);

    valiant::AudioTask::GetSingleton()->Enable(
        valiant::audio::SampleRate::k16000_Hz,
        dma_buffers.data(), dma_buffers.size(), kDmaBufferSize,
    nullptr, +[](void *param, const int32_t* buffer, size_t buffer_size) {
        int32_t offset = g_audio_buffer_end_index;
        for (size_t i = 0; i < buffer_size; ++i)
            g_audio_buffer[(offset + i) % kAudioBufferSize] = buffer[i] >> 16;
        g_audio_buffer_end_index += buffer_size;
    });

    // Fill audio buffer
    vTaskDelay(pdMS_TO_TICKS(kAudioBufferSizeMs));

    setup();
    while (true) {
        loop();
    }
}
