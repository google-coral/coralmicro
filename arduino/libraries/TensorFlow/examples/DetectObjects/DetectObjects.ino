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

// Runs a local server with an endpoint called 'detect_from_camera', which
// will capture an image from the board's camera, run the image through an
// object detection model on the Edge TPU and return the results to a connected
// Python client app through an RPC server.
//
// NOTE: The Python client app works on Windows and Linux only.
// The Python client is available in github.com/google-coral/coralmicro/examples
//
// After uploading the sketch, run this Python client to trigger an inference
// with a photo and receive the results over USB:
//    python3 -m pip install -r examples/detect_camera/requirements.txt
//    python3 examples/detect_camera/detect_camera_client.py

// [start-snippet:ardu-detection]
#include <coralmicro_SD.h>
#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"
#include "coral_micro.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tensorflow/detection.h"

using namespace coralmicro;
using namespace coralmicro::arduino;

namespace {
bool setup_success{false};
int button_pin = PIN_BTN;
int last_button_state = LOW;
int current_button_state = HIGH;
unsigned long last_debounce_time = 0;
constexpr unsigned long kDebounceDelay = 50;

tflite::MicroMutableOpResolver<3> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;
int model_height;
int model_width;

constexpr char kModelPath[] =
    "/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite";
std::vector<tensorflow::Object> results;

constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

FrameBuffer frame_buffer;

bool DetectFromCamera() {
  if (Camera.grab(frame_buffer) != CameraStatus::SUCCESS) {
    return false;
  }
  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor),
              frame_buffer.getBuffer(), frame_buffer.getBufferSize());
  if (interpreter->Invoke() != kTfLiteOk) {
    return false;
  }
  results = tensorflow::GetDetectionResults(interpreter.get(), 0.6f, 3);
  return true;
}

void DetectRpc(struct jsonrpc_request* r) {
  if (!setup_success) {
    jsonrpc_return_error(
        r, -1, "Inference failed because setup was not successful", nullptr);
    return;
  }
  if (!DetectFromCamera()) {
    jsonrpc_return_error(r, -1, "Failed to run classification from camera.",
                         nullptr);
    return;
  }
  if (!results.empty()) {
    const auto& result = results[0];
    jsonrpc_return_success(
        r,
        "{%Q: %d, %Q: %d, %Q: %V, %Q: {%Q: %d, %Q: %g, %Q: %g, %Q: %g, "
        "%Q: %g, %Q: %g}}",
        "width", model_width, "height", model_height, "base64_data",
        frame_buffer.getBufferSize(), frame_buffer.getBuffer(), "detection",
        "id", result.id, "score", result.score, "xmin", result.bbox.xmin,
        "xmax", result.bbox.xmax, "ymin", result.bbox.ymin, "ymax",
        result.bbox.ymax);
    return;
  }
  jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: None}", "width",
                         model_width, "height", model_height, "base64_data",
                         frame_buffer.getBufferSize(), frame_buffer.getBuffer(),
                         "detection");
}
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera Detection!");

  pinMode(button_pin, INPUT);

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
  model_height = input_tensor->dims->data[1];
  model_width = input_tensor->dims->data[2];

  if (Camera.begin(model_width, model_height, coralmicro::CameraFormat::kRgb,
                   coralmicro::CameraFilterMethod::kBilinear,
                   coralmicro::CameraRotation::k270,
                   true) != CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }

  Serial.println("Initializing detection server...");
  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("detect_from_camera", DetectRpc);
  UseHttpServer(new JsonRpcHttpServer);
  Serial.println("Detection server ready!");

  setup_success = true;
  Serial.println("Initialized");
}
void loop() {
  int reading = digitalRead(button_pin);
  if (reading != last_button_state) {
    last_debounce_time = millis();
  }
  if ((millis() - last_debounce_time) > kDebounceDelay) {
    if (reading != current_button_state) {
      current_button_state = reading;
      if (current_button_state == HIGH) {
        Serial.println("Button triggered, running detection...");
        if (!setup_success) {
          Serial.println("Cannot run because of a problem during setup!");
          return;
        }

        if (!DetectFromCamera()) {
          Serial.println("Failed to run detection");
          return;
        }

        Serial.print("Results count: ");
        Serial.println(results.size());
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
    }
  }
  last_button_state = reading;
}
// [end-snippet:ardu-detection]
