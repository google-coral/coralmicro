#ifndef LIBS_POSENET_POSENET_H_
#define LIBS_POSENET_POSENET_H_

#include "third_party/tflite-micro/tensorflow/lite/c/common.h"
#include <cstdint>

namespace coral::micro {
namespace posenet {

constexpr int kPoses = 10;
constexpr int kKeypoints = 17;

constexpr int kPosenetWidth = 324;
constexpr int kPosenetHeight = 324;
constexpr int kPosenetDepth = 3;
constexpr int kPosenetSize = kPosenetWidth * kPosenetHeight * kPosenetDepth;

struct Keypoint {
    float x;
    float y;
    float score;
};

struct Pose {
    float score;
    Keypoint keypoints[kKeypoints];
};

struct Output {
    int num_poses;
    Pose poses[kPoses];
};

bool setup();
bool loop(Output* output);
bool loop(Output* output, bool print);
TfLiteTensor* input();

}  // namespace posenet
}  // namespace coral::micro

#endif  // LIBS_POSENET_POSENET_H_
