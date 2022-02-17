#include "libs/base/tasks.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/audio_provider.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/main_functions.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.h"

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

#include <cstdio>

static int32_t audio_sample_data[2][kMaxAudioSampleSize] __attribute__((section(".noinit.$rpmsg_sh_mem")));
static bool is_audio_initialized = false;

constexpr int kSamplesPerMs = kAudioSampleFrequency / 1000;
constexpr int kAudioCaptureBufferSize = 1 * kAudioSampleFrequency;  // 1 second.
int16_t g_audio_capture_buffer[kAudioCaptureBufferSize];
int16_t g_audio_output_buffer[kMaxAudioSampleSize];
int32_t g_latest_audio_timestamp_ms = 0;

// Based on nxp_k66f implementation
void CaptureSamples(const int32_t *sample_data) {
    const int32_t next_timestamp_ms = g_latest_audio_timestamp_ms + (kMaxAudioSampleSize / kSamplesPerMs);
    const int32_t offset = g_latest_audio_timestamp_ms * kSamplesPerMs;
    for (int i = 0; i < kMaxAudioSampleSize; ++i) {
        g_audio_capture_buffer[(offset + i) % kAudioCaptureBufferSize] = (sample_data[i] >> 16) & 0xFFFF;
    }
    g_latest_audio_timestamp_ms = next_timestamp_ms;
}

int32_t LatestAudioTimestamp() {
    return g_latest_audio_timestamp_ms;
}

TfLiteStatus InitAudioRecording() {
    valiant::AudioTask::GetSingleton()->SetPower(true);
    valiant::AudioTask::GetSingleton()->Enable(valiant::audio::SampleRate::k16000_Hz,
    {audio_sample_data[0], audio_sample_data[1]}, kMaxAudioSampleSize,
    nullptr, +[](void *param, const int32_t* buffer, size_t buffer_size) {
        CaptureSamples(buffer);
    });
    return kTfLiteOk;
}

TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int start_ms, int duration_ms,
                             int* audio_samples_size, int16_t** audio_samples) {
    if (!is_audio_initialized) {
        TfLiteStatus init_status = InitAudioRecording();
        if (init_status != kTfLiteOk) {
            return init_status;
        }
        is_audio_initialized = true;
    }

    const int offset = start_ms * kSamplesPerMs;
    for (int i = 0; i < duration_ms * kSamplesPerMs; ++i) {
        g_audio_output_buffer[i] = g_audio_capture_buffer[(offset + i) % kAudioCaptureBufferSize];
    }
    *audio_samples_size = kMaxAudioSampleSize;
    *audio_samples = g_audio_output_buffer;
    return kTfLiteOk;
}

void RespondToCommand(tflite::ErrorReporter* error_reporter,
                      int32_t current_time, const char* found_command,
                      uint8_t score, bool is_new_command) {
    if (is_new_command) {
        printf("RespondToCommand current_time: %ld found_command %s score: %d new: %d ticks: %ld\r\n",
                current_time, found_command, score, is_new_command, xTaskGetTickCount());
    }
}

extern "C" void app_main(void *param) {
    printf("Micro speech\r\n");
    setup();
    while (true) {
        loop();
        taskYIELD();
    }
}
