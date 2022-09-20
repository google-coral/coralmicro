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

#include <atomic>

#include "libs/audio/audio_driver.h"
#include "model.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"
#include "tensorflow/lite/micro/examples/micro_speech/audio_provider.h"
#include "tensorflow/lite/micro/examples/micro_speech/command_responder.h"
#include "tensorflow/lite/micro/examples/micro_speech/feature_provider.h"
#include "tensorflow/lite/micro/examples/micro_speech/main_functions.h"
#include "tensorflow/lite/micro/examples/micro_speech/micro_features/micro_features_generator.h"
#include "tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.h"
#include "tensorflow/lite/micro/examples/micro_speech/recognize_commands.h"

// Runs a 20 kB TFLM model that can recognize 2 keywords, "yes" and "no",
// using the mic on the Dev Board Micro and printing results to the serial
// console. The model runs on the M7 core alone; it does NOT use the Edge TPU.
//
// For more information about this model, see:
// https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech
//

namespace {
bool setup_success{false};

tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
const tflite::Model* model = nullptr;
std::unique_ptr<tflite::MicroInterpreter> interpreter = nullptr;
tflite::MicroMutableOpResolver<4> micro_op_resolver;
TfLiteTensor* model_input = nullptr;
FeatureProvider* feature_provider = nullptr;
RecognizeCommands* recognizer = nullptr;
int8_t* model_input_buffer = nullptr;
int32_t previous_time = 0;
int8_t feature_buffer[kFeatureElementCount];

constexpr int kSamplesPerMs = kAudioSampleFrequency / 1000;
constexpr int kNumDmaBuffers = 10;
constexpr int kDmaBufferSizeMs = 100;
constexpr int kDmaBufferSize = kDmaBufferSizeMs * kSamplesPerMs;

// Setup audio.
coralmicro::AudioDriverBuffers<kNumDmaBuffers, kNumDmaBuffers * kDmaBufferSize>
    g_audio_buffers;
coralmicro::AudioDriver audio_driver(g_audio_buffers);
coralmicro::AudioDriverConfig audio_config{
    coralmicro::AudioSampleRate::k16000_Hz, kNumDmaBuffers, kDmaBufferSizeMs};

constexpr int kAudioBufferSizeMs = 1000;
constexpr int kAudioBufferSize = kAudioBufferSizeMs * kSamplesPerMs;
int16_t g_audio_buffer[kAudioBufferSize] __attribute__((aligned(16)));
std::atomic<int32_t> g_audio_buffer_end_index = 0;
int16_t g_audio_buffer_out[kMaxAudioSampleSize] __attribute__((aligned(16)));

constexpr int kTensorArenaSize = 10 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
}  // namespace

void RespondToCommand(tflite::ErrorReporter*, int32_t current_time,
                      const char* found_command, uint8_t score,
                      bool is_new_command) {
  if (is_new_command) {
    TF_LITE_REPORT_ERROR(error_reporter, "Heard %s (%d) @%dms", found_command,
                         score, current_time);
    if (strcmp(found_command, "yes") == 0) {
      digitalWrite(PIN_LED_USER, HIGH);
      digitalWrite(PIN_LED_STATUS, LOW);
    } else if (strcmp(found_command, "no") == 0) {
      digitalWrite(PIN_LED_USER, LOW);
      digitalWrite(PIN_LED_STATUS, HIGH);
    } else {
      digitalWrite(PIN_LED_USER, LOW);
      digitalWrite(PIN_LED_STATUS, LOW);
    }
  }
}

TfLiteStatus GetAudioSamples(tflite::ErrorReporter*, int start_ms,
                             int duration_ms, int* audio_samples_size,
                             int16_t** audio_samples) {
  int32_t audio_buffer_end_index = g_audio_buffer_end_index;

  auto buffer_end_ms = audio_buffer_end_index / kSamplesPerMs;
  auto buffer_start_ms = buffer_end_ms - kAudioBufferSizeMs;

  if (start_ms < buffer_start_ms) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "start_ms < buffer_start_ms (%d vs %d)", start_ms,
                         buffer_start_ms);
    return kTfLiteError;
  }

  if (start_ms + duration_ms >= buffer_end_ms) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "start_ms + duration_ms > buffer_end_ms");
    return kTfLiteError;
  }

  int offset =
      audio_buffer_end_index + (start_ms - buffer_start_ms) * kSamplesPerMs;
  for (int i = 0; i < kMaxAudioSampleSize; ++i)
    g_audio_buffer_out[i] = g_audio_buffer[(offset + i) % kAudioBufferSize];

  *audio_samples = g_audio_buffer_out;
  *audio_samples_size = kMaxAudioSampleSize;
  return kTfLiteOk;
}

// From audio_provider.h
int32_t LatestAudioTimestamp() {
  return g_audio_buffer_end_index / kSamplesPerMs - 50;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_USER, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);

  Serial.println("TFLM Micro Speech Arduino!");

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_micro_speech_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.print("Model provided is schema version ");
    Serial.print(model->version());
    Serial.print(" not equal to supported version ");
    Serial.println(TFLITE_SCHEMA_VERSION);
    return;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddSoftmax();
  micro_op_resolver.AddReshape();

  interpreter = std::make_unique<tflite::MicroInterpreter>(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }

  // Get information about the memory area to use for the model's input.
  model_input = interpreter->input(0);
  if ((model_input->dims->size != 2) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] !=
       (kFeatureSliceCount * kFeatureSliceSize)) ||
      (model_input->type != kTfLiteInt8)) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Bad input tensor parameters in model");
    return;
  }
  model_input_buffer = model_input->data.int8;
  //  // Prepare to access the audio spectrograms from a microphone or other
  //  source
  //  // that will provide the inputs to the neural network.
  //  // NOLINTNEXTLINE(runtime-global-variables)
  static FeatureProvider static_feature_provider(kFeatureElementCount,
                                                 feature_buffer);
  feature_provider = &static_feature_provider;

  static RecognizeCommands static_recognizer(error_reporter);
  recognizer = &static_recognizer;

  previous_time = 0;

  audio_driver.Enable(
      audio_config, nullptr,
      +[](void* ctx, const int32_t* buffer, size_t buffer_size) {
        int32_t offset = g_audio_buffer_end_index;
        for (size_t i = 0; i < buffer_size; ++i)
          g_audio_buffer[(offset + i) % kAudioBufferSize] = buffer[i] >> 16;
        g_audio_buffer_end_index += buffer_size;
      });

  delay(kAudioBufferSizeMs);
  setup_success = true;
}

void loop() {
  if (!setup_success) {
    Serial.println(
        "Failed to run inference because there was an error during setup.");
  }
  // Fetch the spectrogram for the current time.
  const int32_t current_time = LatestAudioTimestamp();
  int how_many_new_slices = 0;
  TfLiteStatus feature_status = feature_provider->PopulateFeatureData(
      error_reporter, previous_time, current_time, &how_many_new_slices);
  if (feature_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Feature generation failed");
    return;
  }
  previous_time = current_time;
  // If no new audio samples have been received since last time, don't bother
  // running the network model.
  if (how_many_new_slices == 0) {
    return;
  }

  // Copy feature buffer to input tensor
  for (int i = 0; i < kFeatureElementCount; i++) {
    model_input_buffer[i] = feature_buffer[i];
  }

  // Run the model on the spectrogram input and make sure it succeeds.
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
    return;
  }

  // Obtain a pointer to the output tensor
  TfLiteTensor* output = interpreter->output(0);
  // Determine whether a command was recognized based on the output of inference
  const char* found_command = nullptr;
  uint8_t score = 0;
  bool is_new_command = false;
  TfLiteStatus process_status = recognizer->ProcessLatestResults(
      output, current_time, &found_command, &score, &is_new_command);
  if (process_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "RecognizeCommands::ProcessLatestResults() failed");
    return;
  }
  // Do something based on the recognized command. The default implementation
  // just prints to the error console, but you should replace this with your
  // own function for a real application.
  RespondToCommand(error_reporter, current_time, found_command, score,
                   is_new_command);
}
