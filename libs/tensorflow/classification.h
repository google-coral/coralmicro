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

#ifndef LIBS_TENSORFLOW_CLASSIFICATION_H_
#define LIBS_TENSORFLOW_CLASSIFICATION_H_

#include <limits>
#include <vector>

#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coralmicro::tensorflow {

// Represents a classification result.
struct Class {
  // The class label id.
  int id;
  // The prediction score.
  float score;
};

// Format the Classification outputs into a string.
//
// @param classes All the classification class predictions, as returned by
// `GetClassificationResults()`.
// @return a string with all predictions in a line-delimited list with ids and
// scores for each classification.
std::string FormatClassificationOutput(
    const std::vector<tensorflow::Class>& classes);

// Converts a classification output tensor into a list of ordered classes.
//
// @param scores The dequantized output tensor.
// @param scores_count The number of scores in the output (the size of
//   the output tensor).
// @param threshold The score threshold for results. All returned results have
//   a score greater-than-or-equal-to this value.
// @param top_k The maximum number of predictions to return.
// @returns The top_k Class predictions (id, score), ordered by score
// (first element has the highest score).
std::vector<Class> GetClassificationResults(
    const float* scores, ssize_t scores_count,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

// Gets results from a classification model as a list of ordered classes.
//
// @param interpreter The already-invoked interpreter for your classification
//   model.
// @param threshold The score threshold for results. All returned results have
//   a score greater-than-or-equal-to this value.
// @param top_k The maximum number of predictions to return.
// @returns The top_k Class predictions (id, score), ordered by score
// (first element has the highest score).
std::vector<Class> GetClassificationResults(
    tflite::MicroInterpreter* interpreter,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

// Checks whether an input tensor needs pre-processing for classification.
// @param intput_tensor The tensor intended as input for a classification model.
// @returns True if the input tensor requires normalization AND quantization
//   (you should run ClassificationPreprocess()); false otherwise.
bool ClassificationInputNeedsPreprocessing(const TfLiteTensor& input_tensor);

// Performs normalization and quantization pre-processing on the given tensor.
// @param input_tensor The tensor you want to pre-process for a clasification
//   model.
// @returns True upon success; false if the tensor type is the wrong format.
bool ClassificationPreprocess(TfLiteTensor* input_tensor);

}  // namespace coralmicro::tensorflow

#endif  // LIBS_TENSORFLOW_CLASSIFICATION_H_
