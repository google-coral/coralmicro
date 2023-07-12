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
#include <coral_micro.h>
#include <coralmicro_SD.h>
#include <coralmicro_camera.h>

#include "tensorflow/lite/micro/examples/person_detection/detection_responder.h"
#include "tensorflow/lite/micro/examples/person_detection/image_provider.h"
#include "tensorflow/lite/micro/examples/person_detection/main_functions.h"
#include "tensorflow/lite/micro/examples/person_detection/model_settings.h"

// Runs a 300 kB TFLM model that recognizes people with the camera on the
// Dev Board Micro, printing scores for "person" and "no person" in the serial
// console. The model runs on the M7 core alone; it does NOT use the Edge TPU.
//
// For more information about this model, see:
// https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/person_detection
//

using namespace coralmicro::arduino;

namespace {
bool setup_success{false};

const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
constexpr char kModelPath[] = "/models/person_detect_model.tflite";
TfLiteTensor* input_tensor = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
tflite::MicroMutableOpResolver<5> micro_op_resolver;

constexpr int kTensorArenaSize = 96 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("TFLM Person Detection Arduino!");

  pinMode(PIN_LED_USER, OUTPUT);

  SD.begin();
  Serial.println("Loading model");

  if (!SD.exists(kModelPath)) {
    Serial.println("Failed to read model");
    return;
  }

  SDFile model_file = SD.open(kModelPath);
  uint32_t model_size = model_file.size();
  model_data.resize(model_size);
  if (model_file.read(model_data.data(), model_size) != model_size) {
    Serial.println("Failed to read model");
    return;
  }
  model = tflite::GetModel(model_data.data());
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.print("Model schema version is ");
    Serial.print(model->version());
    Serial.print(" supported is");
    Serial.println(TFLITE_SCHEMA_VERSION);
    return;
  }

  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();

  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }

  input_tensor = interpreter->input_tensor(0);

  if (Camera.begin(kNumRows, kNumCols, coralmicro::CameraFormat::kY8,
                   coralmicro::CameraFilterMethod::kBilinear,
                   coralmicro::CameraRotation::k270,
                   true) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera.");
    return;
  }

  setup_success = true;
}

void loop() {
  if (!setup_success) {
    Serial.println(
        "Failed to run inference because there was an error during setup.");
  }
  if (GetImage(kNumRows, kNumCols, kNumChannels,
               input_tensor->data.int8) != kTfLiteOk) {
    Serial.println("Image capture failed.");
  }

  // Run the model on this input and make sure it succeeds.
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke failed.");
  }

  // Process the inference results.
  auto* output = interpreter->output(0);
  auto person_score = output->data.uint8[kPersonIndex];
  auto no_person_score = output->data.uint8[kNotAPersonIndex];
  RespondToDetection(person_score, no_person_score);
}