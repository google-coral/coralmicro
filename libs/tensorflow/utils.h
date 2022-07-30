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

#ifndef LIBS_TENSORFLOW_UTILS_H_
#define LIBS_TENSORFLOW_UTILS_H_

#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Allocates a uint8_t tensor arena statically in the Dev Board Micro SDRAM
// (max of 64 MB). This is slightly slower than OCRAM due to off-chip I/O
// overhead costs.
// @param name The variable name for this allocation.
// @param size The byte size to allocate. This macro automatically aligns the
// size to 16 bits.
#define STATIC_TENSOR_ARENA_IN_SDRAM(name, size)         \
  static uint8_t name[size] __attribute__((aligned(16))) \
  __attribute__((section(".sdram_bss,\"aw\",%nobits @")))

// Allocates a uint8_t tensor arena statically in the RT1176 on-chip RAM
// (max of 1.25 MB).
// @param name The variable name for this allocation.
// @param size The byte size to allocate. This macro automatically aligns the
// size to 16 bits.
#define STATIC_TENSOR_ARENA_IN_OCRAM(name, size)         \
  static uint8_t name[size] __attribute__((aligned(16))) \
  __attribute__((section(".ocram_bss,\"aw\",%nobits @")))

namespace coralmicro::tensorflow {

// Represents the dimensions of an image.
struct ImageDims {
  // Pixel height.
  int height;
  // Pixel width.
  int width;
  // Channel depth.
  int depth;
};

// Operator == to compares 2 ImageDims object.
inline bool operator==(const ImageDims& a, const ImageDims& b) {
  return a.height == b.height && a.width == b.width && a.depth == b.depth;
}

// Gets an ImageDims's size.
inline int ImageSize(const ImageDims& dims) {
  return dims.height * dims.width * dims.depth;
}

// Resizes a bitmap image.
// @param in_dims The current dimensions for image `uin`.
// @param uin The input image location.
// @param out_dims The desired dimensions for image `uout`.
// @param uout The output image location.
bool ResizeImage(const ImageDims& in_dims, const uint8_t* uin,
                 const ImageDims& out_dims, uint8_t* uout);

// Gets the size of a tensor.
// @param tensor The tensor to get the size.
// @return The size of the tensor.
inline int TensorSize(TfLiteTensor* tensor) {
  int size = 1;
  for (int i = 0; i < tensor->dims->size; ++i) {
    size *= tensor->dims->data[i];
  }
  return size;
}

// Dequantizes data.
//
// @param tensor_size The tensor's size.
// @param tensor_data The tensor's data.
// @param dequant_data The buffer to return the dequantized data to.
// @param scale The scale of the input tensor.
// @param zero_point The zero point of the input tensor.
// @tparam I The data type of tensor_data.
// @tparam O The desired data type of the dequantized data.
// Note: You should instead use `DequantizeTensor()`.
template <typename I, typename O>
void Dequantize(int tensor_size, I* tensor_data, O* dequant_data, float scale,
                float zero_point) {
  for (int i = 0; i < tensor_size; ++i) {
    dequant_data[i] = scale * (tensor_data[i] - zero_point);
  }
}

// Dequantizes a tensor.
//
// @param tensor The tensor to dequantize.
// @return A vector of quantized data.
// @tparam T The desired output type of the dequantized data.
// When using a model adapter API such as `GetClassificationResults()`,
// this dequantization is done for you.
template <typename T>
std::vector<T> DequantizeTensor(TfLiteTensor* tensor) {
  const auto scale = tensor->params.scale;
  const auto zero_point = tensor->params.zero_point;
  std::vector<T> result(TensorSize(tensor));

  if (tensor->type == kTfLiteUInt8) {
    Dequantize(result.size(), tflite::GetTensorData<uint8_t>(tensor),
               result.data(), scale, zero_point);
  } else if (tensor->type == kTfLiteInt8) {
    Dequantize(result.size(), tflite::GetTensorData<int8_t>(tensor),
               result.data(), scale, zero_point);
  } else {
    assert(false);
  }

  return result;
}
}  // namespace coralmicro::tensorflow

#endif  // LIBS_TENSORFLOW_UTILS_H_
