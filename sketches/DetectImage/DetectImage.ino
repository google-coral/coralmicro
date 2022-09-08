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

// Performs object detection with SSD MobileNet, running on the Edge TPU,
// using a local bitmap image as input.
// Results (up to 3 detected objects) are printed to serial console.

#include <coralmicro_SD.h>

#include "Arduino.h"
#include "coral_micro.h"
#include "libs/tensorflow/detection.h"

namespace {
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
constexpr char kModelPath[] =
    "/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite";

std::vector<uint8_t> image_data;
constexpr char kImagePath[] = "/examples/detect_image/cat_300x300.rgb";

std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
tflite::MicroMutableOpResolver<3> resolver;

constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Detect Image!");

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

  Serial.println("Loading Input Image");
  if (!SD.exists(kImagePath)) {
    Serial.println("Image file not found");
    return;
  }
  SDFile image_file = SD.open(kImagePath);
  uint32_t image_size = image_file.size();
  image_data.resize(image_size);
  if (image_file.read(image_data.data(), image_size) != image_size) {
    Serial.println("Error loading image");
  }
  Serial.println("Input image complete");
  model = tflite::GetModel(model_data.data());
  context = coralmicro::EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!context) {
    Serial.println("Failed to get EdgeTpuContext");
    return;
  }
  Serial.println("model and context created");

  resolver.AddDequantize();
  resolver.AddDetectionPostprocess();
  resolver.AddCustom(coralmicro::kCustomOp, coralmicro::RegisterCustomOp());
  tflite::MicroErrorReporter error_reporter;

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
  Serial.println("Initialized");
}

void loop() {
  delay(1000);

  if (!interpreter) {
    Serial.println("Cannot invoke because of a problem during setup!");
    return;
  }

  auto* input_tensor = interpreter->input_tensor(0);
  if (input_tensor->type != kTfLiteUInt8 ||
      input_tensor->bytes != image_data.size()) {
    Serial.println("ERROR: Invalid input tensor size");
    return;
  }

  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image_data.data(),
              image_data.size());

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("ERROR: Invoke() failed");
    return;
  }

  auto results =
      coralmicro::tensorflow::GetDetectionResults(interpreter.get(), 0.6, 3);
  for (auto result : results) {
    Serial.print("id: ");
    Serial.print(result.id);
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
}
