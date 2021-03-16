#include "libs/base/tasks_m7.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/audio_provider.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/main_functions.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.h"

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

#include <cstdio>

static constexpr int kNumberOfBuffers = 2;
static uint32_t audio_sample_data[kNumberOfBuffers][kMaxAudioSampleSize] __attribute__((section(".noinit.$rpmsg_sh_mem")));
static bool is_audio_initialized = false;
static int queued_buffer = 0;

constexpr int kAudioCaptureBufferSize = kAudioSampleFrequency * 0.5;
int16_t g_audio_capture_buffer[kAudioCaptureBufferSize];
int16_t g_audio_output_buffer[kMaxAudioSampleSize];
int32_t g_latest_audio_timestamp = 0;

// Based on nxp_k66f implementation
void CaptureSamples(const int32_t *sample_data) {
    const int32_t time_in_ms = g_latest_audio_timestamp + (kMaxAudioSampleSize / (kAudioSampleFrequency / 1000));
    const int32_t start_sample_offset =
        g_latest_audio_timestamp * (kAudioSampleFrequency / 1000);
    for (int i = 0; i < kMaxAudioSampleSize; ++i) {
        const int capture_index = (start_sample_offset + i) % kAudioCaptureBufferSize;
        g_audio_capture_buffer[capture_index] = (sample_data[i] >> 16) & 0xFFFF;
    }
    g_latest_audio_timestamp = time_in_ms;
}

int32_t LatestAudioTimestamp() {
    return g_latest_audio_timestamp;
}

TfLiteStatus InitAudioRecording() {
    valiant::AudioTask::GetSingleton()->SetCallback([](void *param) {
        CaptureSamples(reinterpret_cast<int32_t*>(audio_sample_data[queued_buffer]));
        queued_buffer++;
        if (queued_buffer == kNumberOfBuffers) {
            queued_buffer = 0;
        }
        return audio_sample_data[queued_buffer];
    }, NULL);
    valiant::AudioTask::GetSingleton()->SetPower(true);
    valiant::AudioTask::GetSingleton()->SetBuffer(reinterpret_cast<uint32_t*>(audio_sample_data[queued_buffer]), sizeof(audio_sample_data[queued_buffer]));
    valiant::AudioTask::GetSingleton()->Enable();
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

    const int start_offset = start_ms * (kAudioSampleFrequency / 1000);
    const int duration_sample_count =
        duration_ms * (kAudioSampleFrequency / 1000);
    for (int i = 0; i < duration_sample_count; ++i) {
        const int capture_index = (start_offset + i) % kAudioCaptureBufferSize;
        g_audio_output_buffer[i] = g_audio_capture_buffer[capture_index];
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

extern "C" void DebugLog(const char *s) {
    printf("%s", s);
}

extern "C" void app_main(void *param) {
    printf("Micro speech\r\n");
    setup();
    while (true) {
        loop();
        taskYIELD();
    }
}
