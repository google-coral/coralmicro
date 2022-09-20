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

// Performs pose estimation with camera images (using the PoseNet model),
// running on the Edge TPU. Scores and keypoint data is printed to the serial
// console.

#include <coralmicro_SD.h>
#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"
#include "coral_micro.h"
#include "libs/tensorflow/posenet.h"
#include "libs/tensorflow/posenet_decoder_op.h"

using namespace coralmicro::arduino;

namespace {
tflite::MicroMutableOpResolver</*tOpsCount=*/2> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> tpu_context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;

constexpr int kModelArenaSize = 1 * 1024 * 1024;
constexpr int kExtraArenaSize = 1 * 1024 * 1024;
constexpr int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

constexpr char kModelPath[] =
    "/models/posenet_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite";
constexpr char kTestInputPath[] = "/models/posenet_test_input_324.bin";
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera Poses!");

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
  tpu_context = coralmicro::EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!tpu_context) {
    Serial.println("Failed to get EdgeTpuContext");
    return;
  }
  Serial.println("model and context created");

  tflite::MicroErrorReporter error_reporter;
  resolver.AddCustom(coralmicro::kCustomOp, coralmicro::RegisterCustomOp());
  resolver.AddCustom(coralmicro::kPosenetDecoderOp,
                     coralmicro::RegisterPosenetDecoderOp());

  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, resolver, tensor_arena, kTensorArenaSize, &error_reporter);

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("allocate tensors failed");
  }

  if (!interpreter) {
    Serial.println("Failed to make interpreter");
    return;
  }

  input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];
  int model_channels = input_tensor->dims->data[3];

  if (input_tensor->type != kTfLiteUInt8 ||
      input_tensor->bytes != model_height * model_width * model_channels) {
    Serial.println("ERROR: Invalid input tensor size");
    return;
  }

  Serial.print("width=");
  Serial.print(model_width);
  Serial.print("; height=");
  Serial.println(model_height);
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
  delay(1000);

  if (!interpreter) {
    Serial.println("Cannot invoke because of a problem during setup!");
    return;
  }

  if (Camera.grab(tflite::GetTensorData<uint8_t>(input_tensor)) !=
      CameraStatus::SUCCESS) {
    Serial.println("cannot invoke because camera failed to grab frame");
    return;
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("ERROR: Invoke() failed");
    return;
  }
  auto poses = coralmicro::tensorflow::GetPosenetOutput(interpreter.get(),
                                                        /*threshold=*/0.5);
  Serial.print("\r\nNum Poses: ");
  Serial.println(poses.size());
  for (size_t i = 0; i < poses.size(); ++i) {
    Serial.print("Pose: ");
    Serial.print(i);
    Serial.print(" -- score: ");
    Serial.println(poses[i].score);
    for (int j = 0; j < coralmicro::tensorflow::kKeypoints; j++) {
      Serial.print("Keypoint: ");
      Serial.print(coralmicro::tensorflow::KeypointTypes[j]);
      auto x = poses[i].keypoints[j].x;
      Serial.print(" -- x=");
      Serial.print(x);
      auto y = poses[i].keypoints[j].y;
      Serial.print(", y=");
      Serial.print(y);
      Serial.print(", score=");
      auto score = poses[i].keypoints[j].score;
      Serial.println(score);
    }
  }
}
