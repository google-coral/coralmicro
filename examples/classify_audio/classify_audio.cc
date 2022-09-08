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
#include "libs/base/led.h"
#include "libs/base/timer.h"
#include "libs/tensorflow/audio_models.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Recognizes various sounds heard with the on-board mic, using YamNet,
// running on the Edge TPU (though it can be modified to run on the CPU).
// See model details here: https://tfhub.dev/google/yamnet/1
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e yamnet

namespace coralmicro {
namespace {

constexpr int kTensorArenaSize = 1 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
constexpr int kNumDmaBuffers = 2;
constexpr int kDmaBufferSizeMs = 50;
constexpr int kDmaBufferSize =
    kNumDmaBuffers * tensorflow::kYamnetSampleRateMs * kDmaBufferSizeMs;
constexpr int kAudioServicePriority = 4;
constexpr int kDropFirstSamplesMs = 150;

AudioDriverBuffers<kNumDmaBuffers, kDmaBufferSize> audio_buffers;
AudioDriver audio_driver(audio_buffers);

constexpr int kAudioBufferSizeMs = tensorflow::kYamnetDurationMs;
constexpr int kAudioBufferSize =
    kAudioBufferSizeMs * tensorflow::kYamnetSampleRateMs;

constexpr float kThreshold = 0.3;
constexpr int kTopK = 5;

std::array<int16_t, tensorflow::kYamnetAudioSize> audio_input;

#ifdef YAMNET_CPU
// To run YamNet on the CPU, see the CMakeLists file to enable this.
constexpr char kModelName[] = "/models/yamnet_spectra_in.tflite";
constexpr bool kUseTpu = false;
#else
constexpr char kModelName[] = "/models/yamnet_spectra_in_edgetpu.tflite";
constexpr bool kUseTpu = true;
#endif

// Run invoke and get the results after the interpreter have already been
// populated with raw audio input.
void run(tflite::MicroInterpreter* interpreter, FrontendState* frontend_state) {
  auto input_tensor = interpreter->input_tensor(0);
  auto preprocess_start = TimerMillis();
  tensorflow::YamNetPreprocessInput(audio_input.data(), input_tensor,
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
      "Yamnet preprocess time: %lums, invoke time: %lums, total: "
      "%lums\r\n",
      static_cast<uint32_t>(preprocess_end - preprocess_start),
      static_cast<uint32_t>(current_time - preprocess_end),
      static_cast<uint32_t>(current_time - preprocess_start));
  auto results =
      tensorflow::GetClassificationResults(interpreter, kThreshold, kTopK);
  printf("%s\r\n", tensorflow::FormatClassificationOutput(results).c_str());
}

[[noreturn]] void Main() {
  printf("YamNet Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  std::vector<uint8_t> yamnet_tflite;
  if (!LfsReadFile(kModelName, &yamnet_tflite)) {
    printf("Failed to load model\r\n");
    vTaskSuspend(nullptr);
  }

  const auto* model = tflite::GetModel(yamnet_tflite.data());
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf("Model schema version is %lu, supported is %d", model->version(),
           TFLITE_SCHEMA_VERSION);
    vTaskSuspend(nullptr);
  }

#ifndef YAMNET_CPU
  auto edgetpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!edgetpu_context) {
    printf("Failed to get TPU context\r\n");
    vTaskSuspend(nullptr);
  }
#endif

  tflite::MicroErrorReporter error_reporter;
  auto yamnet_resolver = tensorflow::SetupYamNetResolver</*tForTpu=*/kUseTpu>();

  tflite::MicroInterpreter interpreter{model, yamnet_resolver, tensor_arena,
                                       kTensorArenaSize, &error_reporter};
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("AllocateTensors failed.\r\n");
    vTaskSuspend(nullptr);
  }

  FrontendState frontend_state{};
  if (!coralmicro::tensorflow::PrepareAudioFrontEnd(
          &frontend_state, coralmicro::tensorflow::AudioModel::kYAMNet)) {
    printf("coralmicro::tensorflow::PrepareAudioFrontEnd() failed.\r\n");
    vTaskSuspend(nullptr);
  }

  // Run tensorflow on test input file.
  std::vector<uint8_t> yamnet_test_input_bin;
  if (!LfsReadFile("/models/yamnet_test_audio.bin", &yamnet_test_input_bin)) {
    printf("Failed to load test input!\r\n");
    vTaskSuspend(nullptr);
  }
  if (yamnet_test_input_bin.size() !=
      tensorflow::kYamnetAudioSize * sizeof(int16_t)) {
    printf("Input audio size doesn't match expected\r\n");
    vTaskSuspend(nullptr);
  }
  auto input_tensor = interpreter.input_tensor(0);
  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor),
              yamnet_test_input_bin.data(), yamnet_test_input_bin.size());
  run(&interpreter, &frontend_state);

  // Setup audio
  AudioDriverConfig audio_config{AudioSampleRate::k16000_Hz, kNumDmaBuffers,
                                 kDmaBufferSizeMs};
  AudioService audio_service(&audio_driver, audio_config, kAudioServicePriority,
                             kDropFirstSamplesMs);
  LatestSamples audio_latest(
      MsToSamples(AudioSampleRate::k16000_Hz, tensorflow::kYamnetDurationMs));
  audio_service.AddCallback(
      &audio_latest,
      +[](void* ctx, const int32_t* samples, size_t num_samples) {
        static_cast<LatestSamples*>(ctx)->Append(samples, num_samples);
        return true;
      });
  // Delay for the first buffers to fill.
  vTaskDelay(pdMS_TO_TICKS(tensorflow::kYamnetDurationMs));
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
#ifndef YAMNET_CPU
    // Delay 975 ms to rate limit the TPU version.
    vTaskDelay(pdMS_TO_TICKS(tensorflow::kYamnetDurationMs));
#endif
  }
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
