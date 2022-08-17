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

#include <cstdio>

#include "examples/tflm_hello_world/hello_world_model.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

// Runs a tiny TFLM model on the M7 core, NOT on the Edge TPU, which simply
// outputs sine wave values over time in the serial console.
//
// For more information about this model, see:
// https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e tflm_hello_world

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                              char* pcTaskName) {
  printf("Stack overflow in %s\r\n", pcTaskName);
}

namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

int inference_count = 0;
const int kInferencesPerCycle = 1000;
const float kXrange = 2.f * 3.14159265359f;

const int kModelArenaSize = 4096;
const int kExtraArenaSize = 4096;
const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));

static void loop() {
  float position = static_cast<float>(inference_count) /
                   static_cast<float>(kInferencesPerCycle);
  float x_val = position * kXrange;

  input->data.f[0] = x_val;

  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x_val: %f",
                         static_cast<double>(x_val));
    return;
  }

  float y_val = output->data.f[0];

  TF_LITE_REPORT_ERROR(error_reporter, "x_val: %f y_val: %f",
                       static_cast<double>(x_val), static_cast<double>(y_val));

  ++inference_count;
  if (inference_count >= kInferencesPerCycle) {
    inference_count = 0;
  }
}

[[noreturn]] void hello_task(void* param) {
  printf("Starting inference task...\r\n");
  while (true) {
    loop();
    taskYIELD();
  }
}

}  // namespace

extern "C" [[noreturn]] void app_main(void* param) {
  (void)param;
  printf("Hello Tensorflow FreeRTOS Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(coralmicro::Led::kStatus, true);

  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  TF_LITE_REPORT_ERROR(error_reporter, "HelloTensorflowFreeRTOS!");

  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model schema version is %d, supported is %d",
                         model->version(), TFLITE_SCHEMA_VERSION);
    vTaskSuspend(nullptr);
  }

  static tflite::MicroMutableOpResolver<3> resolver(error_reporter);
  resolver.AddQuantize();
  resolver.AddDequantize();
  resolver.AddFullyConnected();
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
    vTaskSuspend(nullptr);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  int ret;
  // High water mark testing showed that this task consumes about 218 words.
  // Set our stack size sufficiently large to accomodate.
  ret = xTaskCreate(hello_task, "HelloTask", configMINIMAL_STACK_SIZE * 3,
                    nullptr, configMAX_PRIORITIES - 1, nullptr);
  if (ret != pdPASS) {
    printf("Failed to start HelloTask\r\n");
  }
  while (true) {
    taskYIELD();
  }
}
