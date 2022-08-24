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

// [start-snippet:ardu-classification]
#include <coralmicro_SD.h>
#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"
#include "coral_micro.h"
#include "libs/tensorflow/classification.h"

using namespace coralmicro::arduino;

namespace {

bool setup_success{false};
tflite::MicroMutableOpResolver<3> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;

constexpr char kModelPath[] = "/models/mnv2_324_quant_bayered_edgetpu.tflite";

constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera Classification!");

  SD.begin();

  Serial.println("Loading Model");

  if (!SD.exists(kModelPath)) {
    Serial.println("Model file not found");
    return;
  }

  SDFile model_file = SD.open(kModelPath);
  uint32_t model_size = model_file.size();
  model_data.resize(model_size);
  if (model_file.read(model_data.data(), model_size) != model_size) {
    Serial.print("Error loading model");
    return;
  }

  model = tflite::GetModel(model_data.data());
  context = coralmicro::EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!context) {
    Serial.println("Failed to get EdgeTpuContext");
    return;
  }
  Serial.println("model and context created");

  tflite::MicroErrorReporter error_reporter;
  resolver.AddDequantize();
  resolver.AddDetectionPostprocess();
  resolver.AddCustom(coralmicro::kCustomOp, coralmicro::RegisterCustomOp());

  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, resolver, tensor_arena, kTensorArenaSize, &error_reporter);

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("allocate tensors failed");
    return;
  }

  if (!interpreter) {
    Serial.println("Failed to make interpreter");
    return;
  }
  if (interpreter->inputs().size() != 1) {
    Serial.println("Bad inputs size");
    Serial.println(interpreter->inputs().size());
    return;
  }

  input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];
  int model_channels = input_tensor->dims->data[3];

  Serial.print("width=");
  Serial.print(model_width);
  Serial.print("; height=");
  Serial.println(model_height);
  if (Camera.begin(model_width, model_height, coralmicro::CameraFormat::kRaw,
                   coralmicro::CameraFilterMethod::kBilinear,
                   coralmicro::CameraRotation::k0,
                   true) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }

  setup_success = true;
  Serial.println("Initialized");
}
void loop() {
  if (!setup_success) {
    Serial.println("Cannot run because of a problem during setup!");
  }

  input_tensor = interpreter->input_tensor(0);
  if (Camera.grab(tflite::GetTensorData<uint8_t>(input_tensor)) !=
      CameraStatus::SUCCESS) {
    Serial.println("cannot invoke because camera failed to grab frame");
    return;
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("ERROR: Invoke() failed");
    return;
  }

  auto results = coralmicro::tensorflow::GetClassificationResults(
      interpreter.get(), 0.0, 3);
  Serial.print("Results count: ");
  Serial.println(results.size());
  for (const auto& result : results) {
    Serial.print("Label ID: ");
    Serial.print(result.id);
    Serial.print(" Score: ");
    Serial.println(result.score);
  }
  delay(1000);
}
// [end-snippet:ardu-classification]
