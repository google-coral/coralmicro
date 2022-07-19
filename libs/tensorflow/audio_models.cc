/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libs/tensorflow/audio_models.h"

#include "libs/base/check.h"
#include "libs/base/filesystem.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coralmicro::tensorflow {

bool PrepareAudioFrontEnd(FrontendState* frontend_state,
                          AudioModel model_type) {
  FrontendConfig config{};
  size_t size_ms, step_size_ms;
  int num_channels, sample_rate;
  if (model_type == kYAMNet) {
    size_ms = kYamnetFeatureSliceDurationMs;
    step_size_ms = kYamnetFeatureSliceStrideMs;
    num_channels = kYamnetFeatureSliceSize;
    sample_rate = kYamnetSampleRate;
    config.filterbank.lower_band_limit = 125.0;
    config.filterbank.upper_band_limit = 7500.0;
  } else if (model_type == kKeywordDetector) {
    size_ms = kKeywordDetectorFeatureSliceDurationMs;
    step_size_ms = kKeywordDetectorFeatureSliceStrideMs;
    num_channels = kKeywordDetectorFeatureSliceSize;
    sample_rate = kKeywordDetectorSampleRate;
    config.filterbank.lower_band_limit = 60.0;
    config.filterbank.upper_band_limit = 3800.0;
  } else {
    CHECK(false && "Invalid audio model");
  }
  config.window.size_ms = size_ms;
  config.window.step_size_ms = step_size_ms;
  config.filterbank.num_channels = num_channels;
  config.noise_reduction.smoothing_bits = 10;
  config.noise_reduction.even_smoothing = 0.025;
  config.noise_reduction.odd_smoothing = 0.06;
  config.noise_reduction.min_signal_remaining =
      1.0;  // Use 1.0 to disable reduction.
  config.pcan_gain_control.enable_pcan = 0;
  config.pcan_gain_control.strength = 0.95;
  config.pcan_gain_control.offset = 80.0;
  config.pcan_gain_control.gain_bits = 21;
  config.log_scale.enable_log = 1;
  config.log_scale.scale_shift = 6;
  if (!FrontendPopulateState(&config, frontend_state, sample_rate)) {
    printf("FrontendPopulateState() failed\r\n");
    return false;
  }
  return true;
}

void YamNetPreprocessInput(const int16_t* audio_input,
                           TfLiteTensor* input_tensor,
                           FrontendState* frontend_state) {
  CHECK(input_tensor);
  // Run frontend process for raw audio data.
  // TODO(michaelbrooks): Properly slice the data so that we don't need to
  // re-run the frontend on windows we've already processed.
  std::vector<int16_t> feature_buffer(kYamnetFeatureElementCount);
  PreprocessAudioInput(audio_input, frontend_state, kYAMNet, feature_buffer,
                       kYamnetAudioSize);

  // Converts the int16_t raw_audio input to float spectrogram.
  auto* input = tflite::GetTensorData<float>(input_tensor);
  // Determine the offset and scalar based on the calculated data.
  // TODO(michaelbrooks): This likely isn't needed, the values are always
  // around the same. Can likely hard code.
  constexpr float kExpectedSpectraMax = 3.5f;
  const auto [min, max] =
      std::minmax_element(std::begin(feature_buffer), std::end(feature_buffer));
  int offset = (*max + *min) / 2;
  float scalar = kExpectedSpectraMax / (*max - offset);
  for (int i = 0; i < kYamnetFeatureElementCount; ++i) {
    input[i] = (static_cast<float>(feature_buffer[i]) - offset) * scalar;
  }
}

void KeywordDetectorPreprocessInput(const int16_t* audio_data,
                                    TfLiteTensor* input_tensor,
                                    FrontendState* frontend_state) {
  CHECK(input_tensor);
  // Run frontend process for raw audio data.
  // TODO(michaelbrooks): Properly slice the data so that we don't need to
  // re-run the frontend on windows we've already processed.
  std::vector<int16_t> feature_buffer(kKeywordDetectorFeatureElementCount);
  PreprocessAudioInput(audio_data, frontend_state, kYAMNet, feature_buffer,
                       kKeywordDetectorAudioSize);

  auto* input = tflite::GetTensorData<uint8>(input_tensor);

  const auto [min, max] =
      std::minmax_element(std::begin(feature_buffer), std::end(feature_buffer));

  float scale = static_cast<float>(*max - *min) / 256.0f;

  for (int i = 0; i < kKeywordDetectorFeatureElementCount; ++i) {
    // This conversion allows for requantization from int16 to uint8
    input[i] = static_cast<uint8_t>(
        static_cast<float>(feature_buffer[i] - *min) / scale);
  }
}

void PreprocessAudioInput(const int16_t* audio_data,
                          FrontendState* frontend_state, AudioModel model_type,
                          std::vector<int16_t>& feature_buffer,
                          size_t num_samples) {
  CHECK(frontend_state);
  // Run frontend process for raw audio data.
  // TODO(michaelbrooks): Properly slice the data so that we don't need to
  // re-run the frontend on windows we've already processed.
  size_t num_samples_remaining = num_samples;
  auto* raw_audio = audio_data;
  int count = 0;
  while (num_samples_remaining > 0) {
    size_t num_samples_read;
    auto frontend_output = FrontendProcessSamples(
        frontend_state, raw_audio, num_samples_remaining, &num_samples_read);
    raw_audio += num_samples_read;
    num_samples_remaining -= num_samples_read;
    if (frontend_output.values != nullptr) {
      for (size_t i = 0; i < frontend_output.size; ++i) {
        feature_buffer[count++] = frontend_output.values[i];
      }
    }
  }
}

}  // namespace coralmicro::tensorflow
