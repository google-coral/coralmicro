// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstring>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/tensorflow/classification.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Performs image classification with MobileNet, running on the Edge TPU,
// using a local bitmap image as input.
// The top 3 class predictions are printed to the serial console.

// [start-sphinx-snippet:classify-image]
namespace coralmicro {
constexpr char kModelPath[] =
    "/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite";
constexpr char kImagePath[] = "/examples/classify_image/cat_224x224.rgb";
constexpr int kTensorArenaSize = 1024 * 1024;
struct TensorArena {
  alignas(16) uint8_t data[kTensorArenaSize];
};

void Main() {
  std::vector<uint8_t> model;
  if (!LfsReadFile(kModelPath, &model)) {
    printf("ERROR: Failed to load %s\r\n", kModelPath);
    return;
  }

  std::vector<uint8_t> image;
  if (!LfsReadFile(kImagePath, &image)) {
    printf("ERROR: Failed to load %s\r\n", kImagePath);
    return;
  }

  //! [edgetpu-context] Doxygen snippet for edgetpu_manager.h
  auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!tpu_context) {
    printf("ERROR: Failed to get EdgeTpu context\r\n");
    return;
  }

  tflite::MicroErrorReporter error_reporter;
  tflite::MicroMutableOpResolver<1> resolver;
  resolver.AddCustom(kCustomOp, RegisterCustomOp());

  // As an alternative check STATIC_TENSOR_ARENA_IN_SDRAM macro.
  auto tensor_arena = std::make_unique<TensorArena>();
  tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()), resolver,
                                       tensor_arena->data, kTensorArenaSize,
                                       &error_reporter);
  //! [edgetpu-context] End snippet
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("ERROR: AllocateTensors() failed\r\n");
    return;
  }

  if (interpreter.inputs().size() != 1) {
    printf("ERROR: Model must have only one input tensor\r\n");
    return;
  }

  auto* input_tensor = interpreter.input_tensor(0);
  if (input_tensor->type != kTfLiteUInt8 ||
      input_tensor->bytes != image.size()) {
    printf("ERROR: Invalid input tensor size\r\n");
    return;
  }

  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image.data(),
              image.size());

  if (interpreter.Invoke() != kTfLiteOk) {
    printf("ERROR: Invoke() failed\r\n");
    return;
  }

  auto results = tensorflow::GetClassificationResults(&interpreter, 0.0f, 3);
  for (auto& result : results)
    printf("Label ID: %d Score: %f\r\n", result.id, result.score);
}
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
// [end-sphinx-snippet:classify-image]
