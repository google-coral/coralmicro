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

#ifndef LIBS_YAMNET_YAMNET_H_
#define LIBS_YAMNET_YAMNET_H_

#include <vector>

#include "libs/tensorflow/classification.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

namespace coral::micro {
namespace yamnet {
inline constexpr int kSampleRate = 16000;
inline constexpr int kSampleRateMs = kSampleRate / 1000;
inline constexpr int kDurationMs = 975;
inline constexpr int kAudioSize = kSampleRate * kDurationMs / 1000;
inline constexpr int kFeatureSliceSize = 64;
inline constexpr int kFeatureSliceCount = 96;
inline constexpr int kFeatureElementCount = kFeatureSliceSize * kFeatureSliceCount;
inline constexpr int kFeatureSliceStrideMs = 10;
inline constexpr int kFeatureSliceDurationMs = 25;
inline constexpr int kFeatureSliceSamples = kFeatureSliceDurationMs * kSampleRateMs;

bool setup();
std::optional<const std::vector<tensorflow::Class>> loop(bool print = true);
std::shared_ptr<int16_t[]> audio_input();
}  // namespace yamnet
}  // namespace coral::micro

#endif  // LIBS_YAMNET_YAMNET_H_
