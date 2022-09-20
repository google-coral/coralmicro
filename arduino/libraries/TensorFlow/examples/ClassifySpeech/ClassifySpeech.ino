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

#include <PDM.h>
#include <coralmicro_SD.h>

#include "Arduino.h"
#include "coral_micro.h"
#include "libs/tensorflow/audio_models.h"
#include "libs/tensorflow/classification.h"
#include "third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.h"

namespace {
bool setup_success{false};

std::vector<int32_t> current_samples;
std::vector<int16_t> current_samples_16;
tflite::MicroMutableOpResolver</*tOpsCount=*/1> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
std::vector<std::string> labels;

constexpr int kTensorArenaSize = 1 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

FrontendState frontend_state{};
constexpr float kThreshold = 0.3;
constexpr int kTopK = 5;

constexpr char kLabelPath[] = "/models/labels_gc2.raw.txt";
constexpr char kModelName[] = "/models/voice_commands_v0.7_edgetpu.tflite";
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Classify Speech!");

  SD.begin();
  Mic.begin(coralmicro::tensorflow::kKeywordDetectorSampleRate,
            coralmicro::tensorflow::kKeywordDetectorDurationMs);

  tflite::MicroErrorReporter error_reporter;
  resolver.AddCustom(coralmicro::kCustomOp, coralmicro::RegisterCustomOp());

  Serial.println("Loading Model");

  if (!SD.exists(kModelName) || !SD.exists(kLabelPath)) {
    Serial.println("Model or label file not found");
    return;
  }

  SDFile model_file = SD.open(kModelName);
  uint32_t model_size = model_file.size();
  model_data.resize(model_size);
  if (model_file.read(model_data.data(), model_size) != model_size) {
    Serial.print("Error loading model");
    return;
  }

  std::vector<uint8_t> labels_raw;
  SDFile label_file = SD.open(kLabelPath);
  uint32_t label_size = label_file.size();
  labels_raw.resize(label_size);
  if (label_file.read(labels_raw.data(), label_size) != label_size) {
    Serial.print("Error loading labels");
    return;
  }

  labels.push_back("negative");
  char* label = strtok((char*)(labels_raw.data()), "\n");
  while (label != nullptr) {
    labels.push_back(label);
    label = strtok(nullptr, "\n");
  }

  model = tflite::GetModel(model_data.data());
  context = coralmicro::EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!context) {
    Serial.println("Failed to get EdgeTpuContext");
    return;
  }

  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, resolver, tensor_arena, kTensorArenaSize, &error_reporter);
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed.");
    return;
  }

  if (!coralmicro::tensorflow::PrepareAudioFrontEnd(
          &frontend_state,
          coralmicro::tensorflow::AudioModel::kKeywordDetector)) {
    Serial.println("coralmicro::tensorflow::PrepareAudioFrontEnd() failed.");
    return;
  }

  current_samples.resize(coralmicro::tensorflow::kKeywordDetectorAudioSize);
  current_samples_16.resize(coralmicro::tensorflow::kKeywordDetectorAudioSize);
  setup_success = true;
  Serial.println("Classify Speech Setup Complete\r\n\n");
}

void loop() {
  if (!setup_success) {
    Serial.println("Cannot invoke because of a problem during setup!");
    return;
  }

  if (Mic.available() < coralmicro::tensorflow::kKeywordDetectorAudioSize) {
    return;  // Wait until we have enough data.
  }
  Mic.read(current_samples, coralmicro::tensorflow::kKeywordDetectorAudioSize);

  for (int i = 0; i < coralmicro::tensorflow::kKeywordDetectorAudioSize; ++i) {
    current_samples_16[i] = current_samples[i] >> 16;
  }

  coralmicro::tensorflow::KeywordDetectorPreprocessInput(
      current_samples_16.data(), interpreter->input_tensor(0), &frontend_state);
  // Reset frontend state.
  FrontendReset(&frontend_state);
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Failed to invoke");
    return;
  }

  auto results = coralmicro::tensorflow::GetClassificationResults(
      interpreter.get(), kThreshold, kTopK);
  for (const auto& c : results) {
    if (c.id < labels.size()) {
      Serial.print(labels[c.id].c_str());
      Serial.print(": ");
      Serial.println(c.score);
    }
  }
}