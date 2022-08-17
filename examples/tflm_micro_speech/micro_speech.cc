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

#include <atomic>
#include <cstdio>

#include "libs/audio/audio_driver.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/audio_provider.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/command_responder.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/main_functions.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.h"

// Runs a 20 kB TFLM model that can recognize 2 keywords, "yes" and "no",
// using the mic on the Dev Board Micro and printing results to the serial
// console. The model runs on the M7 core alone; it does NOT use the Edge TPU.
//
// For more information about this model, see:
// https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e tflm_micro_speech

void RespondToCommand(tflite::ErrorReporter* error_reporter,
                      int32_t current_time, const char* found_command,
                      uint8_t score, bool is_new_command) {
  if (is_new_command) {
    TF_LITE_REPORT_ERROR(error_reporter, "Heard %s (%d) @%dms", found_command,
                         score, current_time);
    if (strcmp(found_command, "yes") == 0) {
      LedSet(coralmicro::Led::kUser, true);
      LedSet(coralmicro::Led::kStatus, false);
    } else if (strcmp(found_command, "no") == 0) {
      LedSet(coralmicro::Led::kUser, false);
      LedSet(coralmicro::Led::kStatus, true);
    } else {
      LedSet(coralmicro::Led::kUser, false);
      LedSet(coralmicro::Led::kStatus, false);
    }
  }
}

namespace coralmicro {
namespace {
constexpr int kSamplesPerMs = kAudioSampleFrequency / 1000;

constexpr int kNumDmaBuffers = 10;
constexpr int kDmaBufferSizeMs = 100;
constexpr int kDmaBufferSize = kDmaBufferSizeMs * kSamplesPerMs;
coralmicro::AudioDriverBuffers<kNumDmaBuffers, kNumDmaBuffers * kDmaBufferSize>
    g_audio_buffers;

constexpr int kAudioBufferSizeMs = 1000;
constexpr int kAudioBufferSize = kAudioBufferSizeMs * kSamplesPerMs;
int16_t g_audio_buffer[kAudioBufferSize] __attribute__((aligned(16)));
std::atomic<int32_t> g_audio_buffer_end_index = 0;

int16_t g_audio_buffer_out[kMaxAudioSampleSize] __attribute__((aligned(16)));

[[noreturn]] void Main() {
  printf("Micro Speech Example!\r\n");

  // Setup audio
  AudioDriver driver(g_audio_buffers);
  AudioDriverConfig config{AudioSampleRate::k16000_Hz, kNumDmaBuffers,
                           kDmaBufferSizeMs};
  driver.Enable(
      config, nullptr,
      +[](void* ctx, const int32_t* buffer, size_t buffer_size) {
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
}  // namespace
}  // namespace coralmicro

// From audio_provider.h
int32_t LatestAudioTimestamp() {
  return coralmicro::g_audio_buffer_end_index / coralmicro::kSamplesPerMs - 50;
}

// From audio_provider.h
TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int start_ms, int duration_ms,
                             int* audio_samples_size, int16_t** audio_samples) {
  int32_t audio_buffer_end_index = coralmicro::g_audio_buffer_end_index;

  auto buffer_end_ms = audio_buffer_end_index / coralmicro::kSamplesPerMs;
  auto buffer_start_ms = buffer_end_ms - coralmicro::kAudioBufferSizeMs;

  if (start_ms < buffer_start_ms) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "start_ms < buffer_start_ms (%d vs %d)", start_ms,
                         buffer_start_ms);
    return kTfLiteError;
  }

  if (start_ms + duration_ms >= buffer_end_ms) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "start_ms + duration_ms > buffer_end_ms");
    return kTfLiteError;
  }

  int offset = audio_buffer_end_index +
               (start_ms - buffer_start_ms) * coralmicro::kSamplesPerMs;
  for (int i = 0; i < kMaxAudioSampleSize; ++i)
    coralmicro::g_audio_buffer_out[i] =
        coralmicro::g_audio_buffer[(offset + i) % coralmicro::kAudioBufferSize];

  *audio_samples = coralmicro::g_audio_buffer_out;
  *audio_samples_size = kMaxAudioSampleSize;
  return kTfLiteOk;
}

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
