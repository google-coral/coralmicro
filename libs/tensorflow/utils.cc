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

#include "libs/tensorflow/utils.h"

#include "third_party/tflite-micro/tensorflow/lite/micro/kernels/kernel_runner.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/test_helpers.h"

namespace coralmicro::tensorflow {

bool ResizeImage(const ImageDims& in_dims, const uint8_t* uin,
                 const ImageDims& out_dims, uint8_t* uout) {
  if (in_dims == out_dims) {
    memcpy(uout, uin, ImageSize(in_dims));
    return true;
  }

  // TODO(dkovalev): Implement resizing in a more efficient way. TFLM doesn't
  // support uint8, int8 is required for all operations.
  const auto in_size = ImageSize(in_dims);
  auto in_tmp = std::make_unique<int8_t[]>(in_size);
  int8_t* in = in_tmp.get();

  const auto out_size = ImageSize(out_dims);
  auto out_tmp = std::make_unique<int8_t[]>(out_size);
  int8_t* out = out_tmp.get();

  for (int i = 0; i < in_size; ++i) in[i] = static_cast<int>(uin[i]) - 128;

  // Array with 4 values: batch, rows, columns, depth
  int input_dims_ints[] = {4, 1, in_dims.height, in_dims.width, in_dims.depth};
  TfLiteIntArray* input_dims =
      tflite::testing::IntArrayFromInts(input_dims_ints);
  // Array with 1 value: the number of dimensions.
  int size_dims_ints[] = {1, 2};
  TfLiteIntArray* size_dims = tflite::testing::IntArrayFromInts(size_dims_ints);
  // Array with 4 values: batch, rows, columns, depth
  int output_dims_ints[] = {4, 1, out_dims.height, out_dims.width,
                            out_dims.depth};
  TfLiteIntArray* output_dims =
      tflite::testing::IntArrayFromInts(output_dims_ints);

  constexpr int tensors_size = 3;
  int32_t expected_size[] = {out_dims.height, out_dims.width};
  TfLiteTensor tensors[tensors_size] = {
      tflite::testing::CreateQuantizedTensor(in, input_dims, 0, 255),
      tflite::testing::CreateTensor(expected_size, size_dims),
      tflite::testing::CreateQuantizedTensor(out, output_dims, 0, 255)};
  tensors[1].allocation_type = kTfLiteMmapRo;

  // Array with 2 values, representing the indices of the input tensors in the
  // tensors array.
  int inputs_ints[] = {2, 0, 1};
  TfLiteIntArray* inputs = tflite::testing::IntArrayFromInts(inputs_ints);
  // Array with 1 value, representing the indices of the output tensors in the
  // tensors array.
  int outputs_ints[] = {1, 2};
  TfLiteIntArray* outputs = tflite::testing::IntArrayFromInts(outputs_ints);

  TfLiteResizeNearestNeighborParams params = {false, /* align_corners */
                                              false /* half_pixel_centers */};

  tflite::micro::KernelRunner runner(
      tflite::ops::micro::Register_RESIZE_NEAREST_NEIGHBOR(), tensors,
      tensors_size, inputs, outputs, &params);

  if (runner.InitAndPrepare() != kTfLiteOk) {
    printf("failed to init and prepare\r\n");
    return false;
  }

  if (runner.Invoke() != kTfLiteOk) {
    printf("failed to invoke\r\n");
    return false;
  }

  for (int i = 0; i < out_size; ++i) uout[i] = static_cast<int>(out[i]) + 128;

  return true;
}

}  // namespace coralmicro::tensorflow
