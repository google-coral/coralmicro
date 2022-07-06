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

// Allocate tensor arena statically in SDRAM.
#define STATIC_TENSOR_ARENA_IN_SDRAM(name, size)           \
    static uint8_t name[size] __attribute__((aligned(16))) \
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")))

// Allocate tensor arena statically in on-chip RAM.
#define STATIC_TENSOR_ARENA_IN_OCRAM(name, size)           \
    static uint8_t name[size] __attribute__((aligned(16))) \
    __attribute__((section(".ocram_bss,\"aw\",%nobits @")))

namespace coralmicro {
namespace tensorflow {

// Represents the dimensions of an image.
struct ImageDims {
    // Pixel height.
    int height;
    // Pixel width.
    int width;
    // Channel depth.
    int depth;
};

inline bool operator==(const ImageDims& a, const ImageDims& b) {
    return a.height == b.height && a.width == b.width && a.depth == b.depth;
}
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
inline int TensorSize(TfLiteTensor* tensor) {
    int size = 1;
    for (int i = 0; i < tensor->dims->size; ++i) {
        size *= tensor->dims->data[i];
    }
    return size;
}

// Dequantizes data.
//
// You should instead use `DequantizeTensor()`.
template <typename I, typename O>
void Dequantize(int count, I* tensor_data, O* dequant_data, float scale,
                float zero_point) {
    for (int i = 0; i < count; ++i) {
        dequant_data[i] = scale * (tensor_data[i] - zero_point);
    }
}

// Dequantizes a tensor.
//
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
}  // namespace tensorflow
}  // namespace coralmicro

#endif  // LIBS_TENSORFLOW_UTILS_H_
