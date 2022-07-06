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

#ifndef LIBS_TPU_EDGETPU_OP_H_
#define LIBS_TPU_EDGETPU_OP_H_

#include "tensorflow/lite/c/common.h"

namespace coralmicro {

// Edge TPU custom op. Pass this to
// `tflite::MicroMutableOpResolver::AddCustom()`.
inline constexpr char kCustomOp[] = "edgetpu-custom-op";

// Returns pointer to an instance of `tflite::TfLiteRegistration` to handle
// Edge TPU custom ops. Pass this to
// `tflite::MicroMutableOpResolver::AddCustom()`.
TfLiteRegistration* RegisterCustomOp();

}  // namespace coralmicro

#endif  // LIBS_TPU_EDGETPU_OP_H_
