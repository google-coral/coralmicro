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

#include "libs/audio/audio_service.h"
#include "libs/base/filesystem.h"
#include "libs/base/timer.h"
#include "libs/tensorflow/audio_models.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace coralmicro {
namespace {
constexpr int kTensorArenaSize = 1 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
constexpr int kNumDmaBuffers = 2;
constexpr int kDmaBufferSizeMs = 50;
constexpr int kDmaBufferSize = kNumDmaBuffers *
                               tensorflow::kKeywordDetectorSampleRateMs *
                               kDmaBufferSizeMs;
constexpr int kAudioServicePriority = 4;
constexpr int kDropFirstSamplesMs = 150;

AudioDriverBuffers<kNumDmaBuffers, kDmaBufferSize> audio_buffers;
AudioDriver audio_driver(audio_buffers);

constexpr int kAudioBufferSizeMs = tensorflow::kKeywordDetectorDurationMs;
constexpr int kAudioBufferSize =
    kAudioBufferSizeMs * tensorflow::kKeywordDetectorSampleRateMs;

constexpr float kThreshold = 0.3;
constexpr int kTopK = 5;
constexpr char kModelName[] = "/models/voice_commands_v0.7_edgetpu.tflite";
constexpr char kLabelsName[] = "/models/labels_gc2.raw.txt";

std::array<int16_t, tensorflow::kKeywordDetectorAudioSize> audio_input;
std::vector<std::string> labels;

// Run invoke and get the results after the interpreter have already been
// populated with raw audio input.
void run(tflite::MicroInterpreter* interpreter, FrontendState* frontend_state) {
  auto input_tensor = interpreter->input_tensor(0);
  auto preprocess_start = TimerMillis();
  tensorflow::KeywordDetectorPreprocessInput(audio_input.data(), input_tensor,
                                             frontend_state);
  // Reset frontend state.
  FrontendReset(frontend_state);
  auto preprocess_end = TimerMillis();
  if (interpreter->Invoke() != kTfLiteOk) {
    printf("Failed to invoke on test input\r\n");
    vTaskSuspend(nullptr);
  }

  auto current_time = TimerMillis();
  printf(
      "Keyword Detector preprocess time: %lums, invoke time: %lums, total: "
      "%lums\r\n",
      static_cast<uint32_t>(preprocess_end - preprocess_start),
      static_cast<uint32_t>(current_time - preprocess_end),
      static_cast<uint32_t>(current_time - preprocess_start));
  auto results =
      tensorflow::GetClassificationResults(interpreter, kThreshold, kTopK);
  for (const auto& c : results) {
    if ((size_t)c.id < labels.size()) {
      printf("%s\r\n", (labels[c.id] + ": " + std::to_string(c.score)).c_str());
    }
  }
  printf("\r\n");
}

}  // namespace

[[noreturn]] void Main() {
  printf("Keyword Detector!!!\r\n");
  std::vector<uint8_t> keyword_tflite;
  if (!LfsReadFile(kModelName, &keyword_tflite)) {
    printf("Failed to load model\r\n");
    vTaskSuspend(nullptr);
  }

  std::vector<uint8_t> labels_raw;
  if (!LfsReadFile(kLabelsName, &labels_raw)) {
    printf("Failed to load labels\r\n");
    vTaskSuspend(nullptr);
  }

  labels.push_back("negative");
  char* label = strtok((char*)(labels_raw.data()), "\n");
  while (label != nullptr) {
    labels.push_back(label);
    label = strtok(nullptr, "\n");
  }

  auto edgetpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!edgetpu_context) {
    printf("Failed to get TPU context\r\n");
    vTaskSuspend(nullptr);
  }

  tflite::MicroErrorReporter error_reporter;
  tflite::MicroMutableOpResolver<1> resolver;
  resolver.AddCustom(kCustomOp, RegisterCustomOp());

  tflite::MicroInterpreter interpreter(tflite::GetModel(keyword_tflite.data()),
                                       resolver, tensor_arena, kTensorArenaSize,
                                       &error_reporter);

  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("AllocateTensors failed.\r\n");
    vTaskSuspend(nullptr);
  }

  FrontendState frontend_state{};
  if (!tensorflow::PrepareAudioFrontEnd(
          &frontend_state, tensorflow::AudioModel::kKeywordDetector)) {
    printf("tensorflow::PrepareAudioFrontEnd() failed.\r\n");
    vTaskSuspend(nullptr);
  }

  // Setup audio
  AudioDriverConfig audio_config{AudioSampleRate::k16000_Hz, kNumDmaBuffers,
                                 kDmaBufferSizeMs};
  AudioService audio_service(&audio_driver, audio_config, kAudioServicePriority,
                             kDropFirstSamplesMs);
  LatestSamples audio_latest(MsToSamples(
      AudioSampleRate::k16000_Hz, tensorflow::kKeywordDetectorDurationMs));
  audio_service.AddCallback(
      &audio_latest,
      +[](void* ctx, const int32_t* samples, size_t num_samples) {
        static_cast<LatestSamples*>(ctx)->Append(samples, num_samples);
        return true;
      });

  // Delay for the first buffers to fill.
  vTaskDelay(pdMS_TO_TICKS(tensorflow::kKeywordDetectorDurationMs));

  while (true) {
    audio_latest.AccessLatestSamples(
        [](const std::vector<int32_t>& samples, size_t start_index) {
          size_t i, j = 0;
          // Starting with start_index, grab until the end of the buffer.
          for (i = 0; i < samples.size() - start_index; ++i) {
            audio_input[i] = samples[i + start_index] >> 16;
          }
          // Now fill the rest of the data with the beginning of the
          // buffer.
          for (j = 0; j < samples.size() - i; ++j) {
            audio_input[i + j] = samples[j] >> 16;
          }
        });
    run(&interpreter, &frontend_state);

    // Delay 2000ms to rate limit the TPU version.
    vTaskDelay(pdMS_TO_TICKS(tensorflow::kKeywordDetectorDurationMs));
  }
}
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}