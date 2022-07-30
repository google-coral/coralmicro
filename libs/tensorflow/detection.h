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

#ifndef LIBS_TENSORFLOW_DETECTION_H_
#define LIBS_TENSORFLOW_DETECTION_H_

#include <limits>
#include <vector>

#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coralmicro::tensorflow {

// Represents the bounding box of a detected object.
template <typename T>
struct BBox {
  // The box y-minimum (top-most) point.
  T ymin;
  // The box x-minimum (left-most) point.
  T xmin;
  // The box y-maximum (bottom-most) point.
  T ymax;
  // The box x-maximum (right-most) point.
  T xmax;
};

// Represents a detected object.
struct Object {
  // The class label id.
  int id;
  // The prediction score.
  float score;
  // The bounding-box (ymin,xmin,ymax,xmax).
  BBox<float> bbox;
};

// Formats the detection outputs into a string.
//
// @param object A vector with all the objects in an object detection
// output.
// @return A description of all detected objects.
std::string FormatDetectionOutput(const std::vector<Object>& objects);

// Converts detection output tensors into a vector of Objects.
//
// @param bboxes The output tensor for all detected bounding boxes in
//  box-corner encoding, for example:
//  (ymin1,xmin1,ymax1,xmax1,ymin2,xmin2,...).
// @param ids The output tensor for all label IDs.
// @param scores The output tensor for all scores.
// @param count The number of detected objects (all tensors defined above
//   have valid data for this number of objects).
// @param threshold The score threshold for results. All returned results have
//   a score greater-than-or-equal-to this value.
// @param top_k The maximum number of predictions to return.
// @returns The top_k object predictions (id, score, BBox), ordered by score
// (first element has the highest score).
std::vector<Object> GetDetectionResults(
    const float* bboxes, const float* ids, const float* scores, size_t count,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

// Gets results from a detection model as a vector of Objects.
//
// @param interpreter The already-invoked interpreter for your detection model.
// @param threshold The score threshold for results. All returned results have
//   a score greater-than-or-equal-to this value.
// @param top_k The maximum number of predictions to return.
// @returns The top_k object predictions (id, score, BBox), ordered by score
// (first element has the highest score).
std::vector<Object> GetDetectionResults(
    tflite::MicroInterpreter* interpreter,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

}  // namespace coralmicro::tensorflow

#endif  // LIBS_TENSORFLOW_DETECTION_H_
