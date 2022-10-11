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

#ifndef LIBS_YAMNET_YAMNET_H_
#define LIBS_YAMNET_YAMNET_H_

#include <vector>

#include "libs/tensorflow/classification.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"
#include "third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace coralmicro::tensorflow {

// Supported models.
enum AudioModel {
  // YamNet without the frontend.
  kYAMNet,
  // Keyword detector (or "Keyword Spotter").
  kKeywordDetector,
};

inline constexpr int kYamnetSampleRate = 16000;
inline constexpr int kYamnetSampleRateMs = kYamnetSampleRate / 1000;
inline constexpr int kYamnetDurationMs = 975;
inline constexpr int kYamnetAudioSize =
    kYamnetSampleRate * kYamnetDurationMs / 1000;
inline constexpr int kYamnetFeatureSliceSize = 64;
inline constexpr int kYamnetFeatureSliceCount = 96;
inline constexpr int kYamnetFeatureElementCount =
    (kYamnetFeatureSliceSize * kYamnetFeatureSliceCount);
inline constexpr int kYamnetFeatureSliceStrideMs = 10;
inline constexpr int kYamnetFeatureSliceDurationMs = 25;

inline constexpr int kKeywordDetectorSampleRate = 16000;
inline constexpr int kKeywordDetectorSampleRateMs =
    kKeywordDetectorSampleRate / 1000;
inline constexpr int kKeywordDetectorDurationMs = 2000;
inline constexpr int kKeywordDetectorAudioSize =
    kKeywordDetectorSampleRate * kKeywordDetectorDurationMs / 1000;
inline constexpr int kKeywordDetectorFeatureSliceSize = 32;
inline constexpr int kKeywordDetectorFeatureSliceCount = 198;
inline constexpr int kKeywordDetectorFeatureElementCount =
    (kKeywordDetectorFeatureSliceSize * kKeywordDetectorFeatureSliceCount);
inline constexpr int kKeywordDetectorFeatureSliceStrideMs = 10;
inline constexpr int kKeywordDetectorFeatureSliceDurationMs = 25;

// Sets up the MicroMutableOpResolver with ops required for YamNet.
//
// @tparam tForTpu If true the Resolver will be setup for TPU else CPU.
// @return A tflite::MicroMutableOpResolver that is prepared for the YamNet
// model.
template <bool tForTpu>
auto SetupYamNetResolver() {
  constexpr unsigned int kNumOps = tForTpu ? 3 : 9;
  auto resolver = tflite::MicroMutableOpResolver<kNumOps>();
  resolver.AddQuantize();
  resolver.AddDequantize();
  if constexpr (tForTpu) {
    resolver.AddCustom(coralmicro::kCustomOp, coralmicro::RegisterCustomOp());
  } else {
    resolver.AddReshape();
    resolver.AddSplit();
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddLogistic();
    resolver.AddMean();
    resolver.AddFullyConnected();
  }
  return resolver;
}

// Prepares the input preprocess engine for TensorFlow to converts raw audio
// data to spectrogram. This function must be called before
// `PreprocessAudioInput()` is called.
//
// @param frontend_state The FrontendState struct to populate.
// @param model_type The type of audio model.
// @return true on `FrontendPopulateState()` success, else false.
bool PrepareAudioFrontEnd(FrontendState* frontend_state, AudioModel model_type);

// Performs input preprocessing to convert raw input to spectrogram.
//
// @param audio_data An array of signed int16 audio data.
// @param input_tensor The tensor where the preprocessed spectrogram data
// is stored.
// @param frontend_state The populated frontend state that you want to
// preprocess the input tensor, must not be nullptr.
void YamNetPreprocessInput(const int16_t* audio_data,
                           TfLiteTensor* input_tensor,
                           FrontendState* frontend_state);

// Performs input preprocessing to convert raw audio input to spectrogram.
//
// @param audio_data An array of signed int16 audio data.
// @param input_tensor The tensor you want to pre-process for a TensorFlow
// model, must not be nullptr.
// @param frontend_state The populated frontend state that you want to
// preprocess the input tensor, must not be nullptr.
void KeywordDetectorPreprocessInput(const int16_t* audio_data,
                                    TfLiteTensor* input_tensor,
                                    FrontendState* frontend_state);

// @cond
void PreprocessAudioInput(const int16_t* audio_data,
                          FrontendState* frontend_state, AudioModel model_type,
                          std::vector<int16_t>& feature_buffer,
                          size_t num_samples);
// @endcond

}  // namespace coralmicro::tensorflow

#endif  // LIBS_YAMNET_YAMNET_H_
