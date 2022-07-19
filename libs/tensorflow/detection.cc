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

#include "libs/tensorflow/detection.h"

#include <cmath>
#include <queue>

namespace coralmicro::tensorflow {

namespace {
struct ObjectComparator {
  bool operator()(const Object& lhs, const Object& rhs) const {
    return std::tie(lhs.score, lhs.id) > std::tie(rhs.score, rhs.id);
  }
};
}  // namespace

std::string FormatDetectionOutput(const std::vector<Object>& objects) {
  std::string output;
  for (const auto& object : objects) {
    output += "id: " + std::to_string(object.id) +
              " -- score: " + std::to_string(object.score) +
              " -- xmin: " + std::to_string(object.bbox.xmin) +
              " -- ymin: " + std::to_string(object.bbox.ymin) +
              " -- xmax: " + std::to_string(object.bbox.xmax) +
              " -- ymax: " + std::to_string(object.bbox.ymax);
  }
  return output;
}

std::vector<Object> GetDetectionResults(const float* bboxes, const float* ids,
                                        const float* scores, size_t count,
                                        float threshold, size_t top_k) {
  std::priority_queue<Object, std::vector<Object>, ObjectComparator> q;

  for (unsigned int i = 0; i < count; ++i) {
    const int id = std::round(ids[i]);
    const float score = scores[i];
    const float ymin = std::max(0.0f, bboxes[4 * i]);
    const float xmin = std::max(0.0f, bboxes[4 * i + 1]);
    const float ymax = std::max(0.0f, bboxes[4 * i + 2]);
    const float xmax = std::max(0.0f, bboxes[4 * i + 3]);
    if (score < threshold) {
      continue;
    }
    q.push(Object{id, score, BBox<float>{ymin, xmin, ymax, xmax}});
    if (q.size() > top_k) {
      q.pop();
    }
  }

  std::vector<Object> ret;
  ret.reserve(q.size());
  while (!q.empty()) {
    ret.push_back(q.top());
    q.pop();
  }
  std::reverse(ret.begin(), ret.end());
  return ret;
}

std::vector<Object> GetDetectionResults(tflite::MicroInterpreter* interpreter,
                                        float threshold, size_t top_k) {
  if (interpreter->outputs().size() != 4) {
    printf("Output size mismatch\r\n");
    return {};
  }

  const float *bboxes, *ids, *scores, *count;
  if (interpreter->output_tensor(2)->dims->size == 1) {
    scores = tflite::GetTensorData<float>(interpreter->output_tensor(0));
    bboxes = tflite::GetTensorData<float>(interpreter->output_tensor(1));
    count = tflite::GetTensorData<float>(interpreter->output_tensor(2));
    ids = tflite::GetTensorData<float>(interpreter->output_tensor(3));
  } else {
    bboxes = tflite::GetTensorData<float>(interpreter->output_tensor(0));
    ids = tflite::GetTensorData<float>(interpreter->output_tensor(1));
    scores = tflite::GetTensorData<float>(interpreter->output_tensor(2));
    count = tflite::GetTensorData<float>(interpreter->output_tensor(3));
  }

  return GetDetectionResults(bboxes, ids, scores, static_cast<size_t>(count[0]),
                             threshold, top_k);
}

}  // namespace coralmicro::tensorflow
