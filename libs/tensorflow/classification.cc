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

#include "libs/tensorflow/classification.h"

#include <queue>
#include <vector>

#include "libs/tensorflow/utils.h"

namespace coralmicro::tensorflow {
namespace {
// Defines a comparator which allows us to rank Class based on their score and
// id.
struct ClassComparator {
  bool operator()(const Class& lhs, const Class& rhs) const {
    return std::tie(lhs.score, lhs.id) > std::tie(rhs.score, rhs.id);
  }
};
}  // namespace

std::string FormatClassificationOutput(
    const std::vector<tensorflow::Class>& classes) {
  if (classes.empty()) {
    return "No results\r\n";
  }
  std::string out;
  out += "Results:\r\n";
  for (const auto& c : classes) {
    out += std::to_string(c.id) + ": " + std::to_string(c.score) + "\r\n";
  }
  return out;
}

std::vector<Class> GetClassificationResults(const float* scores,
                                            ssize_t scores_count,
                                            float threshold, size_t top_k) {
  std::priority_queue<Class, std::vector<Class>, ClassComparator> q;
  for (int i = 0; i < scores_count; ++i) {
    if (scores[i] < threshold) continue;
    q.push(Class{i, scores[i]});
    if (q.size() > top_k) q.pop();
  }

  std::vector<Class> ret;
  while (!q.empty()) {
    ret.push_back(q.top());
    q.pop();
  }
  std::reverse(ret.begin(), ret.end());
  return ret;
}

std::vector<Class> GetClassificationResults(
    tflite::MicroInterpreter* interpreter, float threshold, size_t top_k) {
  auto tensor = interpreter->output_tensor(0);
  if (tensor->type == kTfLiteUInt8 || tensor->type == kTfLiteInt8) {
    auto scores = DequantizeTensor<float>(tensor);
    return GetClassificationResults(scores.data(), scores.size(), threshold,
                                    top_k);
  } else if (tensor->type == kTfLiteFloat32) {
    auto scores = tflite::GetTensorData<float>(tensor);
    return GetClassificationResults(scores, TensorSize(tensor), threshold,
                                    top_k);
  } else {
    assert(false);
  }
}

bool ClassificationInputNeedsPreprocessing(const TfLiteTensor& input_tensor) {
  const float scale = input_tensor.params.scale;
  const float zero_point = input_tensor.params.zero_point;
  const float mean = 128;
  const float std = 128;
  const float epsilon = 1e-5;
  if (std::abs(scale * std - 1) < epsilon &&
      std::abs(mean - zero_point) < epsilon) {
    return false;
  } else {
    return true;
  }
}

bool ClassificationPreprocess(TfLiteTensor* input_tensor) {
  if (input_tensor->type != kTfLiteUInt8) {
    return false;
  }
  const float scale = input_tensor->params.scale;
  const float zero_point = input_tensor->params.zero_point;
  const float mean = 128;
  const float std = 128;
  const size_t size = input_tensor->bytes;

  uint8_t* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
  for (size_t i = 0; i < size; ++i) {
    const float tmp =
        (input_tensor_data[i] - mean) / (std * scale) + zero_point;
    if (tmp > 255) {
      input_tensor_data[i] = 255;
    } else if (tmp < 0) {
      input_tensor_data[i] = 0;
    } else {
      input_tensor_data[i] = static_cast<uint8_t>(tmp);
    }
  }
  return true;
}

}  // namespace coralmicro::tensorflow
