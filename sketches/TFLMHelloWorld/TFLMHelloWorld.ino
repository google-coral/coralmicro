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

#include <Arduino.h>

#include "coral_micro.h"
#include "hello_world_model.h"

// Runs a tiny TFLM model on the M7 core, NOT on the Edge TPU, which simply
// outputs sine wave values over time in the serial console.
//
// For more information about this model, see:
// https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples
//

namespace {
bool setup_success{false};

tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
tflite::MicroMutableOpResolver<3> resolver;
const tflite::Model* model = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

int inference_count = 0;
const int kInferencesPerCycle = 1000;
const float kXrange = 2.f * 3.14159265359f;

const int kModelArenaSize = 4096;
const int kExtraArenaSize = 4096;
const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Hello Tensorflow Arduino!");

  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model schema version is %d, supported is %d",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  resolver.AddQuantize();
  resolver.AddDequantize();
  resolver.AddFullyConnected();

  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
    return;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  setup_success = true;
}

void loop() {
  if (!setup_success) {
    Serial.println(
        "Failed to run inference because there was an error during setup.");
  }

  float position = static_cast<float>(inference_count) /
                   static_cast<float>(kInferencesPerCycle);
  float x_val = position * kXrange;

  input->data.f[0] = x_val;
  if (interpreter->Invoke() != kTfLiteOk) {
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