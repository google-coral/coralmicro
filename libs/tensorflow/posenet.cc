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

#include "libs/tensorflow/posenet.h"

#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coralmicro::tensorflow {

std::string FormatPosenetOutput(const std::vector<Pose>& poses) {
  std::string out;
  out += "Num Poses: " + std::to_string(poses.size()) + "\r\n";
  for (size_t i = 0; i < poses.size(); ++i) {
    float pose_score = poses[i].score;
    out += "Pose " + std::to_string(i) +
           " -- score: " + std::to_string(pose_score) + "\r\n";
    for (int j = 0; j < kKeypoints; ++j) {
      float y = poses[i].keypoints[j].y;
      float x = poses[i].keypoints[j].x;
      float score = poses[i].keypoints[j].score;
      out += std::string("Keypoint ") + KeypointTypes[j] +
             " -- x=" + std::to_string(x) + ",y=" + std::to_string(y) +
             ",score=" + std::to_string(score) + "\r\n";
    }
  }
  return out;
}

std::vector<Pose> GetPosenetOutput(tflite::MicroInterpreter* interpreter,
                                   float threshold) {
  auto* keypoints = tflite::GetTensorData<float>(interpreter->output(0));
  auto* keypoints_scores = tflite::GetTensorData<float>(interpreter->output(1));
  auto* pose_scores = tflite::GetTensorData<float>(interpreter->output(2));
  auto* num_poses = tflite::GetTensorData<float>(interpreter->output(3));
  int pose_count = static_cast<int>(num_poses[0]);
  std::vector<Pose> poses;
  poses.reserve(pose_count);
  for (int i = 0; i < pose_count; ++i) {
    if (pose_scores[i] < threshold)
      continue;  // Skip poses that are less than the threshold.
    Pose pose{};
    pose.score = pose_scores[i];
    float* pose_keypoints = keypoints + (i * kKeypoints * 2);
    float* keypoint_scores = keypoints_scores + (i * kKeypoints);
    for (int j = 0; j < kKeypoints; ++j) {
      float* point = pose_keypoints + (j * 2);
      pose.keypoints[j].x = point[1];
      pose.keypoints[j].y = point[0];
      pose.keypoints[j].score = keypoint_scores[j];
    }
    poses.push_back(pose);
  }
  return poses;
}

}  // namespace coralmicro::tensorflow
