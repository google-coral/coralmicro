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

// Recognizes various sounds heard with the on-board mic, using YamNet,
// running on the Edge TPU (though it can be modified to run on the CPU).
// See model details here: https://tfhub.dev/google/yamnet/1

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
tflite::MicroMutableOpResolver</*tOpsCount=*/3> resolver;
const tflite::Model* model = nullptr;
std::vector<uint8_t> model_data;
std::shared_ptr<coralmicro::EdgeTpuContext> context = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
std::array<int16_t, coralmicro::tensorflow::kYamnetAudioSize> audio_input;

constexpr int kTensorArenaSize = 1 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

FrontendState frontend_state{};
constexpr float kThreshold = 0.3;
constexpr int kTopK = 5;

constexpr char kModelName[] = "/models/yamnet_spectra_in_edgetpu.tflite";
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino YamNet!");

  SD.begin();
  Mic.begin(coralmicro::tensorflow::kYamnetSampleRate,
            coralmicro::tensorflow::kYamnetDurationMs);

  tflite::MicroErrorReporter error_reporter;
  resolver = coralmicro::tensorflow::SetupYamNetResolver</*tForTpu=*/true>();

  Serial.println("Loading Model");

  if (!SD.exists(kModelName)) {
    Serial.println("Model file not found");
    return;
  }

  SDFile model_file = SD.open(kModelName);
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

  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, resolver, tensor_arena, kTensorArenaSize, &error_reporter);
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed.");
    return;
  }

  if (!coralmicro::tensorflow::PrepareAudioFrontEnd(
          &frontend_state, coralmicro::tensorflow::AudioModel::kYAMNet)) {
    Serial.println("coralmicro::tensorflow::PrepareAudioFrontEnd() failed.");
    return;
  }

  setup_success = true;
  Serial.println("YAMNet Setup Complete\r\n\n");
}

void loop() {
  if (!setup_success) {
    Serial.println("Cannot invoke because of a problem during setup!");
    return;
  }

  if (Mic.available() < coralmicro::tensorflow::kYamnetAudioSize) {
    return;  // Wait until we have enough data.
  }
  Mic.read(current_samples, coralmicro::tensorflow::kYamnetAudioSize);

  for (int i = 0; i < coralmicro::tensorflow::kYamnetAudioSize; ++i) {
    audio_input[i] = current_samples[i] >> 16;
  }

  coralmicro::tensorflow::YamNetPreprocessInput(
      audio_input.data(), interpreter->input_tensor(0), &frontend_state);
  // Reset frontend state.
  FrontendReset(&frontend_state);
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Failed to invoke");
    return;
  }

  auto results = coralmicro::tensorflow::GetClassificationResults(
      interpreter.get(), kThreshold, kTopK);
  if (results.empty()) {
    Serial.println("No results");
  } else {
    Serial.println("Results");
    for (const auto& c : results) {
      Serial.print(c.id);
      Serial.print(": ");
      Serial.println(c.score);
    }
  }
}
