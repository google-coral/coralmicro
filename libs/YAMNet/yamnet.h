#ifndef LIBS_YAMNET_YAMNET_H_
#define LIBS_YAMNET_YAMNET_H_

#include <vector>

#include "libs/tensorflow/classification.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

namespace coral::micro {
namespace yamnet {
constexpr int kSampleRate = 16000;
constexpr int kSampleRateMs = kSampleRate / 1000;
constexpr int kDurationMs = 975;
constexpr int kAudioSize = kSampleRate * kDurationMs / 1000;
constexpr int kFeatureSliceSize = 64;
constexpr int kFeatureSliceCount = 96;
constexpr int kFeatureElementCount = (kFeatureSliceSize * kFeatureSliceCount);
constexpr int kFeatureSliceStrideMs = 10;
constexpr int kFeatureSliceDurationMs = 25;
constexpr int kFeatureSliceSamples = kFeatureSliceDurationMs * kSampleRateMs;

bool setup();
std::optional<const std::vector<tensorflow::Class>> loop(bool print = true);
std::shared_ptr<int16_t[]> audio_input();
}  // namespace yamnet
}  // namespace coral::micro

#endif  // LIBS_YAMNET_YAMNET_H_
