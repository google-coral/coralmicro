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

// Runs body segmentation using BodyPix on the Edge TPU, returning one result
// at a time. Inferencing starts only when an RPC client requests the
// `run_bodypix` endpoint. The model receives continuous input from the camera
// until the model returns a good result. At which point, the result is sent to
// the RPC client and inferencing stops.
//
// Then trigger an inference over USB from a Linux computer using the same
// client that we used for the equivalent FreeRTOS version (examples/bodypix):
//    python3 -m pip install -r examples/bodypix/requirements.txt
//    python3 examples/bodypix/bodypix_client.py

#include <Arduino.h>
#include <coral_micro.h>
#include <coralmicro_SD.h>
#include <coralmicro_camera.h>

#include "libs/base/utils.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tensorflow/posenet.h"
#include "libs/tensorflow/posenet_decoder_op.h"

namespace {
using namespace coralmicro;
using namespace coralmicro::arduino;

bool setup_success{false};

tflite::MicroMutableOpResolver<2> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;
int model_height;
int model_width;

constexpr int kTensorArenaSize = 2 * 1024 * 1024;
constexpr float kConfidenceThreshold = 0.5f;
constexpr int kLowConfidenceResult = -2;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
constexpr char kModelPath[] =
    "/models/bodypix_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite";

FrameBuffer frame_buffer;

void RunBodypix(struct jsonrpc_request* r) {
  if (!setup_success) {
    jsonrpc_return_error(
        r, -1, "Inference failed because setup was not successful", nullptr);
    return;
  }

  auto* bodypix_input = interpreter->input(0);

  std::vector<tensorflow::Pose> results;
  if (Camera.grab(frame_buffer) != CameraStatus::SUCCESS) {
    Serial.println("cannot invoke because camera failed to grab frame");
    jsonrpc_return_error(r, -1, "Failed to get image from camera.", nullptr);
    return;
  }
  std::memcpy(tflite::GetTensorData<uint8_t>(bodypix_input),
              frame_buffer.getBuffer(), frame_buffer.getBufferSize());
  if (interpreter->Invoke() != kTfLiteOk) {
    jsonrpc_return_error(r, -1, "Invoke failed", nullptr);
    return;
  }
  results =
      tensorflow::GetPosenetOutput(interpreter.get(), kConfidenceThreshold);
  if (results.empty()) {
    jsonrpc_return_error(r, kLowConfidenceResult, "below confidence threshold", nullptr);
    return;
  }

  const auto& float_segments_tensor = interpreter->output_tensor(5);
  const auto& float_segments =
      tflite::GetTensorData<uint8_t>(float_segments_tensor);
  const auto segment_size = tensorflow::TensorSize(float_segments_tensor);
  jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: %V}", "width",
                         model_width, "height", model_height, "base64_data",
                         frame_buffer.getBufferSize(), frame_buffer.getBuffer(),
                         "output_mask1", segment_size, float_segments);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Bodypix!");

  SD.begin();

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
  resolver.AddCustom(coralmicro::kPosenetDecoderOp,
                     coralmicro::RegisterPosenetDecoderOp());
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

  input_tensor = interpreter->input_tensor(0);
  model_height = input_tensor->dims->data[1];
  model_width = input_tensor->dims->data[2];
  if (Camera.begin(model_width, model_height, coralmicro::CameraFormat::kRgb,
                   coralmicro::CameraFilterMethod::kBilinear,
                   coralmicro::CameraRotation::k270,
                   true) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }

  Serial.println("Initializing Bodypix server...");
  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("run_bodypix", RunBodypix);
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);

  Serial.println("Bodypix Server Ready!");
  setup_success = true;
}

void loop() {
  if (!setup_success) {
    Serial.println(
        "Server not started because there was an error during setup");
    delay(10000);
  }
}
