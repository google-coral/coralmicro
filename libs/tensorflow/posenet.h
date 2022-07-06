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

#ifndef LIBS_POSENET_POSENET_H_
#define LIBS_POSENET_POSENET_H_

#include "libs/tensorflow/posenet_decoder_op.h"
#include "libs/tensorflow/utils.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coralmicro::tensorflow {

// Number of keypoints in each pose.
inline constexpr int kKeypoints = 17;

// A map of keypoint index to the keypoint name.
inline const char* const KeypointTypes[] = {
    "NOSE",        "LEFT_EYE",      "RIGHT_EYE",      "LEFT_EAR",
    "RIGHT_EAR",   "LEFT_SHOULDER", "RIGHT_SHOULDER", "LEFT_ELBOW",
    "RIGHT_ELBOW", "LEFT_WRIST",    "RIGHT_WRIST",    "LEFT_HIP",
    "RIGHT_HIP",   "LEFT_KNEE",     "RIGHT_KNEE",     "LEFT_ANKLE",
    "RIGHT_ANKLE",
};

// Represents a keypoint of a pose object.
struct Keypoint {
    // The keypoint's x position relative to the model size.
    float x;
    // The keypoint's y position relative to the model size.
    float y;
    // The keypoint's score.
    float score;
};

// Represents a pose in the output object.
struct Pose {
    // The pose's score.
    float score;
    // An array of keypoints in this pose.
    Keypoint keypoints[kKeypoints];
};

// Format the posenet outputs into a string.
//
// @param poses a vector contains all the poses in a posenet output.
// score is under the threshold.
// @return A string showing the posenet's output.
std::string FormatPosenetOutput(const std::vector<Pose>& poses);

// Gets the results from a posenet model in the form of a vector of poses.
//
// @param interpreter The already-invoked interpreter for your posenet model.
// @param threshold The score threshold for results. All returned
// poses results have  a score greater-than-or-equal-to this value.
// @param print Prints the posenet results before returning if set to true.
std::vector<Pose> GetPosenetOutput(
    tflite::MicroInterpreter* interpreter,
    float threshold = -std::numeric_limits<float>::infinity(),
    bool print = false);

}  // namespace coralmicro::tensorflow

#endif  // LIBS_POSENET_POSENET_H_
