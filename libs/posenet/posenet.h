#ifndef _LIBS_POSENET_H_
#define _LIBS_POSENET_H_

#include "third_party/tensorflow/tensorflow/lite/c/common.h"
#include <cstdint>

namespace valiant {
namespace posenet {

constexpr int kPoses = 10;
constexpr int kKeypoints = 17;

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
TfLiteTensor* input();

}  // namespace posenet
}  // namespace valiant

#endif  // _LIBS_POSENET_H_