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

// Runs a local RPC server with an endpoint called 'segment_from_camera',
// which will capture an image from the board's camera, run the image through a
// segmentation model and return the results to a connected
// Python client app through an RPC server.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e segment_camera
//
// NOTE: The Python client app works on Windows and Linux only.
// The Python client is available in github.com/google-coral/coralmicro/examples
//
// After uploading the sketch, run this Python client to trigger an inference
// with a photo and receive the results over USB:
//    python3 -m pip install -r examples/segment_camera/requirements.txt
//    python3 examples/segment_camera/segment_camera_client.py

#include <Arduino.h>
#include <coral_micro.h>
#include <coralmicro_SD.h>
#include <coralmicro_camera.h>

#include "libs/base/utils.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"

namespace {
using namespace coralmicro;
using namespace coralmicro::arduino;

bool setup_success{false};

tflite::MicroMutableOpResolver<3> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;
int model_height;
int model_width;

constexpr int kTensorArenaSize = 2 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
constexpr char kModelPath[] =
    "/models/keras_post_training_unet_mv2_128_quant_edgetpu.tflite";

FrameBuffer frame_buffer;

void SegmentFromCamera(struct jsonrpc_request* r) {
  if (!setup_success) {
    jsonrpc_return_error(
        r, -1, "Inference failed because setup was not successful", nullptr);
    return;
  }
  if (Camera.grab(frame_buffer) != CameraStatus::SUCCESS) {
    Serial.println("cannot invoke because camera failed to grab frame");
    jsonrpc_return_error(r, -1, "Failed to get image from camera.", nullptr);
    return;
  }
  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor),
              frame_buffer.getBuffer(), frame_buffer.getBufferSize());
  if (interpreter->Invoke() != kTfLiteOk) {
    jsonrpc_return_error(r, -1, "Invoke failed", nullptr);
    return;
  }

  const auto& output_tensor = interpreter->output_tensor(0);
  const auto& output_mask = tflite::GetTensorData<uint8_t>(output_tensor);
  const auto mask_size = tensorflow::TensorSize(output_tensor);
  jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: %V}", "width",
                         model_width, "height", model_height, "base64_data",
                         frame_buffer.getBufferSize(), frame_buffer.getBuffer(),
                         "output_mask", mask_size, output_mask);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera Segmentation!");

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
  resolver.AddResizeBilinear();
  resolver.AddArgMax();
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

  Serial.println("Initializing segmentation server...");
  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("segment_from_camera", SegmentFromCamera);
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);

  Serial.println("Segmentation Server Ready!");
  setup_success = true;
}

void loop() {
  if (!setup_success) {
    Serial.println(
        "Server not started because there was an error during setup");
    delay(10000);
  }
}
