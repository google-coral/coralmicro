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

#include "libs/tensorflow/yamnet.h"

#include "libs/base/check.h"
#include "libs/base/filesystem.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coralmicro::tensorflow {

bool YamNetPrepareFrontEnd(FrontendState* frontend_state) {
  FrontendConfig config{};
  config.window.size_ms = kYamnetFeatureSliceDurationMs;
  config.window.step_size_ms = kYamnetFeatureSliceStrideMs;
  config.filterbank.num_channels = kYamnetFeatureSliceSize;
  config.filterbank.lower_band_limit = 125.0;
  config.filterbank.upper_band_limit = 7500.0;
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
  if (!FrontendPopulateState(&config, frontend_state, kYamnetSampleRate)) {
    printf("FrontendPopulateState() failed\r\n");
    return false;
  }
  return true;
}

void YamNetPreprocessInput(TfLiteTensor* input_tensor,
                           FrontendState* frontend_state) {
  CHECK(input_tensor);
  CHECK(frontend_state);
  // Run frontend process for raw audio data.
  // TODO(michaelbrooks): Properly slice the data so that we don't need to
  // re-run the frontend on windows we've already processed.
  constexpr float kExpectedSpectraMax = 3.5f;
  std::vector<int16_t> feature_buffer(kYamnetFeatureElementCount);
  size_t num_samples_remaining = kYamnetAudioSize;
  auto* raw_audio = tflite::GetTensorData<int16_t>(input_tensor);
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

  // Converts the int16_t raw_audio input to float spectrogram.
  auto* input = tflite::GetTensorData<float>(input_tensor);
  // Determine the offset and scalar based on the calculated data.
  // TODO(michaelbrooks): This likely isn't needed, the values are always
  // around the same. Can likely hard code.
  auto [min, max] =
      std::minmax_element(std::begin(feature_buffer), std::end(feature_buffer));
  int offset = (*max + *min) / 2;
  float scalar = kExpectedSpectraMax / (*max - offset);
  for (int i = 0; i < kYamnetFeatureElementCount; ++i) {
    input[i] = (static_cast<float>(feature_buffer[i]) - offset) * scalar;
  }
}

}  // namespace coralmicro::tensorflow
