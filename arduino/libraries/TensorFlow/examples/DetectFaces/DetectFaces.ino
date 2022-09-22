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

// Runs face detection on the Edge TPU, using the on-board camera, printing
// results to the serial console and turning on the User LED when a face
// is detected.

#include <coralmicro_SD.h>
#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"
#include "coral_micro.h"
#include "libs/tensorflow/detection.h"

using namespace coralmicro::arduino;

namespace {
tflite::MicroMutableOpResolver<3> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;

constexpr char kModelPath[] =
    "/models/ssd_mobilenet_v2_face_quant_postprocess_edgetpu.tflite";
constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Face Detection!");

  pinMode(PIN_LED_USER, OUTPUT);

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
  if (Camera.begin(model_width, model_height, coralmicro::CameraFormat::kRgb,
                   coralmicro::CameraFilterMethod::kBilinear,
                   coralmicro::CameraRotation::k270,
                   true) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }

  Serial.println("Initialized");
}
void loop() {
  input_tensor = interpreter->input_tensor(0);
  if (Camera.grab(tflite::GetTensorData<uint8_t>(input_tensor)) !=
      CameraStatus::SUCCESS) {
    Serial.println("cannot invoke because camera failed to grab frame");
    return;
  }
  if (!interpreter) {
    Serial.println("Cannot invoke because of a problem during setup!");
    return;
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("ERROR: Invoke() failed");
    return;
  }

  auto results =
      coralmicro::tensorflow::GetDetectionResults(interpreter.get(), 0.6, 3);
  if (!results.empty()) {
    digitalWrite(PIN_LED_USER, HIGH);
    Serial.print("Face count: ");
    Serial.println(results.size());
    for (auto result : results) {
      Serial.print(" score: ");
      Serial.print(result.score);
      Serial.print(" xmin: ");
      Serial.print(result.bbox.xmin);
      Serial.print(" ymin: ");
      Serial.print(result.bbox.ymin);
      Serial.print(" xmax: ");
      Serial.print(result.bbox.xmax);
      Serial.print(" ymax: ");
      Serial.println(result.bbox.ymax);
    }
  } else {
    digitalWrite(PIN_LED_USER, LOW);
    Serial.println("No faces found");
  }
}
