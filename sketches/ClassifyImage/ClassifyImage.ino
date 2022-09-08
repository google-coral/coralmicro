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

// Performs image classification with MobileNet, running on the Edge TPU,
// using a local bitmap image as input.
// The top 3 class predictions are printed to the serial console.

#include <coralmicro_SD.h>

#include "Arduino.h"
#include "coral_micro.h"
#include "libs/tensorflow/classification.h"

namespace {
tflite::MicroMutableOpResolver<1> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::vector<uint8_t> image_data;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;

const int kTensorArenaSize = 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Classify Image!");

  SD.begin();

  Serial.println("Loading Model");

  const char* model_name = "/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite";
  if (!SD.exists(model_name)) {
    Serial.println("Model file not found");
    return;
  }

  SDFile model_file = SD.open(model_name);
  uint32_t model_size = model_file.size();
  model_data.resize(model_size);
  if (model_file.read(model_data.data(), model_size) != model_size) {
    Serial.print("Error loading model");
  }

  const char* image_name = "/examples/classify_image/cat_224x224.rgb";
  Serial.println("Loading Input Image");
  if (!SD.exists(image_name)) {
    Serial.println("Image file not found");
    return;
  }
  SDFile image_file = SD.open(image_name);
  uint32_t image_size = image_file.size();
  image_data.resize(image_size);
  if (image_file.read(image_data.data(), image_size) != image_size) {
    Serial.print("Error loading image");
  }

  model = tflite::GetModel(model_data.data());
  context = coralmicro::EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!context) {
    Serial.println("Failed to get EdgeTpuContext");
    return;
  }

  resolver.AddCustom(coralmicro::kCustomOp, coralmicro::RegisterCustomOp());
  tflite::MicroErrorReporter error_reporter;
  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, resolver, tensor_arena, kTensorArenaSize, &error_reporter);
  interpreter->AllocateTensors();

  if (!interpreter) {
    Serial.println("Failed to make interpreter");
    return;
  }
  if (interpreter->inputs().size() != 1) {
    Serial.println("Bad inputs size");
    return;
  }
  Serial.println("Initialized");
}

void loop() {
  delay(1000);

  if (!interpreter) {
    Serial.println("Cannot invoke because of a problem during setup!");
    return;
  }

  auto* input_tensor = interpreter->input_tensor(0);
  if (input_tensor->type != kTfLiteUInt8) {
    Serial.println("Bad input type");
    return;
  }

  if (coralmicro::tensorflow::ClassificationInputNeedsPreprocessing(
          *input_tensor)) {
    coralmicro::tensorflow::ClassificationPreprocess(input_tensor);
  }

  coralmicro::tensorflow::TensorSize(input_tensor);
  auto* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
  memcpy(input_tensor_data, image_data.data(), input_tensor->bytes);

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("invoke failed");
    return;
  }

  auto results = coralmicro::tensorflow::GetClassificationResults(
      interpreter.get(), 0.0f, 3);
  for (auto result : results) {
    Serial.print("Label ID: ");
    Serial.print(result.id);
    Serial.print(" Score: ");
    Serial.println(result.score);
  }
}
