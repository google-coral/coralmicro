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

#include "libs/tpu/edgetpu_op.h"

#include "libs/tpu/edgetpu_manager.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

namespace coralmicro {
namespace {
void* CustomOpInit(TfLiteContext* context, const char* buffer, size_t length) {
  return EdgeTpuManager::GetSingleton()->RegisterPackage(buffer, length);
}

void CustomOpFree(TfLiteContext* context, void* buffer) {}

TfLiteStatus CustomOpPrepare(TfLiteContext* context, TfLiteNode* node) {
  if (node->user_data == nullptr) return kTfLiteError;
  return kTfLiteOk;
}

TfLiteStatus CustomOpInvoke(TfLiteContext* context, TfLiteNode* node) {
  EdgeTpuPackage* package = static_cast<EdgeTpuPackage*>(node->user_data);
  return EdgeTpuManager::GetSingleton()->Invoke(package, context, node);
}
}  // namespace

TfLiteRegistration* RegisterCustomOp() {
  static TfLiteRegistration registration = {
      CustomOpInit,
      CustomOpFree,
      CustomOpPrepare,
      CustomOpInvoke,
  };
  return &registration;
}
}  // namespace coralmicro
