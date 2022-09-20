
// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "libs/base/filesystem.h"
#include "libs/base/led.h"
#include "libs/camera/camera.h"
#include "libs/tensorflow/posenet.h"
#include "libs/tensorflow/posenet_decoder_op.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

// Performs pose estimation with camera images (using the PoseNet model),
// running on the Edge TPU. Scores and keypoint data is printed to the serial
// console.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e posenet

// [start-sphinx-snippet:posenet]
namespace coralmicro {
namespace {

constexpr int kModelArenaSize = 1 * 1024 * 1024;
constexpr int kExtraArenaSize = 1 * 1024 * 1024;
constexpr int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
constexpr char kModelPath[] =
    "/models/posenet_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite";
constexpr char kTestInputPath[] = "/models/posenet_test_input_324.bin";

void Main() {
  printf("Posenet Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  tflite::MicroErrorReporter error_reporter;
  TF_LITE_REPORT_ERROR(&error_reporter, "Posenet!");
  // Turn on the TPU and get it's context.
  auto tpu_context =
      EdgeTpuManager::GetSingleton()->OpenDevice(PerformanceMode::kMax);
  if (!tpu_context) {
    printf("ERROR: Failed to get EdgeTpu context\r\n");
    vTaskSuspend(nullptr);
  }
  // Reads the model and checks version.
  std::vector<uint8_t> posenet_tflite;
  if (!LfsReadFile(kModelPath, &posenet_tflite)) {
    TF_LITE_REPORT_ERROR(&error_reporter, "Failed to load model!");
    vTaskSuspend(nullptr);
  }
  auto* model = tflite::GetModel(posenet_tflite.data());
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(&error_reporter,
                         "Model schema version is %d, supported is %d",
                         model->version(), TFLITE_SCHEMA_VERSION);
    vTaskSuspend(nullptr);
  }
  // Creates a micro interpreter.
  tflite::MicroMutableOpResolver<2> resolver;
  resolver.AddCustom(kCustomOp, RegisterCustomOp());
  resolver.AddCustom(kPosenetDecoderOp, RegisterPosenetDecoderOp());
  auto interpreter = tflite::MicroInterpreter{
      model, resolver, tensor_arena, kTensorArenaSize, &error_reporter};
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(&error_reporter, "AllocateTensors failed.");
    vTaskSuspend(nullptr);
  }
  auto* posenet_input = interpreter.input(0);
  // Runs posenet on a test image.
  printf("Getting outputs for posenet test input\r\n");
  std::vector<uint8_t> posenet_test_input_bin;
  if (!LfsReadFile(kTestInputPath, &posenet_test_input_bin)) {
    TF_LITE_REPORT_ERROR(&error_reporter, "Failed to load test input!");
    vTaskSuspend(nullptr);
  }
  if (posenet_input->bytes != posenet_test_input_bin.size()) {
    TF_LITE_REPORT_ERROR(&error_reporter,
                         "Input tensor length doesn't match canned input\r\n");
    vTaskSuspend(nullptr);
  }
  memcpy(tflite::GetTensorData<uint8_t>(posenet_input),
         posenet_test_input_bin.data(), posenet_test_input_bin.size());
  if (interpreter.Invoke() != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(&error_reporter, "Invoke failed.");
    vTaskSuspend(nullptr);
  }
  auto test_image_output =
      tensorflow::GetPosenetOutput(&interpreter, /*threshold=*/0.5);
  printf("%s\r\n", tensorflow::FormatPosenetOutput(test_image_output).c_str());
  // Starts the camera for live poses.
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);
  printf("Starting live posenet\r\n");
  auto model_height = posenet_input->dims->data[1];
  auto model_width = posenet_input->dims->data[2];
  for (;;) {
    CameraFrameFormat fmt{
        /*fmt=*/CameraFormat::kRgb,
        /*filter=*/CameraFilterMethod::kBilinear,
        /*rotation=*/CameraRotation::k270,
        /*width=*/model_width,
        /*height=*/model_height,
        /*preserve_ratio=*/false,
        /*buffer=*/tflite::GetTensorData<uint8_t>(posenet_input)};
    if (!CameraTask::GetSingleton()->GetFrame({fmt})) {
      TF_LITE_REPORT_ERROR(&error_reporter, "Failed to get image from camera.");
      break;
    }
    if (interpreter.Invoke() != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(&error_reporter, "Invoke failed.");
      break;
    }
    auto output = tensorflow::GetPosenetOutput(&interpreter,
                                               /*threshold=*/0.5);
    printf("%s\r\n", tensorflow::FormatPosenetOutput(output).c_str());
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  CameraTask::GetSingleton()->SetPower(false);
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
// [end-sphinx-snippet:posenet]
