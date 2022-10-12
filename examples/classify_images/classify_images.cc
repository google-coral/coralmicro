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

#include <cstring>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Runs a local server with an endpoint called 'classification_from_camera',
// which will capture an image from the board's camera, run the image through an
// image classification model on the Edge TPU and return the results to a
// connected Python client app through an RPC server.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e classify_images
//
// NOTE: The Python client app works on Windows and Linux only.
//
// After flashing the example, run this Python client to trigger an inference
// with a photo and receive the results over USB:
//    python3 -m pip install -r examples/classify_images/requirements.txt
//    python3 examples/classify_images/classify_images_client.py
//
// The response includes only the top result with a JSON file like this:
//
// {
// 'id': int,
// 'result':
//     {
//     'width': int,
//     'height': int,
//     'base64_data': image_bytes,
//     'bayered': bayered,
//     'id': id,
//     'score': score,
// }

namespace coralmicro {
namespace {
const std::string kModelPath = "/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite";
constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

bool ClassifyFromCamera(tflite::MicroInterpreter* interpreter, int model_width,
                        int model_height, bool bayered,
                        std::vector<tensorflow::Class>* results,
                        std::vector<uint8>* image) {
  CHECK(results != nullptr);
  CHECK(image != nullptr);
  auto* input_tensor = interpreter->input_tensor(0);

  // Note if the model is bayered, the raw data will not be rotated.
  auto format = bayered ? CameraFormat::kRaw : CameraFormat::kRgb;
  CameraFrameFormat fmt{format,
                        CameraFilterMethod::kBilinear,
                        CameraRotation::k270,
                        model_width,
                        model_height,
                        false,
                        image->data()};

  CameraTask::GetSingleton()->Trigger();
  if (!CameraTask::GetSingleton()->GetFrame({fmt})) return false;

  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image->data(),
              image->size());
  if (interpreter->Invoke() != kTfLiteOk) return false;

  *results = tensorflow::GetClassificationResults(interpreter, 0.0f, 1);
  return true;
}

void ClassifyRpc(struct jsonrpc_request* r) {
  auto* interpreter =
      reinterpret_cast<tflite::MicroInterpreter*>(r->ctx->response_cb_data);
  auto* input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];
  // If the model name includes "bayered", provide the raw datastream from the
  // camera.
  auto bayered = kModelPath.find("bayered") != std::string::npos;
  std::vector<uint8_t> image(
      model_width * model_height *
      /*channels=*/(bayered ? 1 : CameraFormatBpp(CameraFormat::kRgb)));
  std::vector<tensorflow::Class> results;
  if (ClassifyFromCamera(interpreter, model_width, model_height, bayered,
                         &results, &image)) {
    if (!results.empty()) {
      const auto& result = results[0];
      jsonrpc_return_success(
          r, "{%Q: %d, %Q: %d, %Q: %V, %Q: %d, %Q: %d, %Q: %g}", "width",
          model_width, "height", model_height, "base64_data", image.size(),
          image.data(), "bayered", bayered, "id", result.id, "score",
          result.score);
      return;
    }
    jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: %d}", "width",
                           model_width, "height", model_height, "base64_data",
                           image.size(), image.data(), "bayered", bayered);
    return;
  }
  jsonrpc_return_error(r, -1, "Failed to classify image from camera.", nullptr);
}

void ClassifyConsole(tflite::MicroInterpreter* interpreter) {
  auto* input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];
  // If the model name includes "bayered", provide the raw datastream from the
  // camera.
  auto bayered = kModelPath.find("bayered") != std::string::npos;
  std::vector<uint8_t> image(
      model_width * model_height *
      /*channels=*/(bayered ? 1 : CameraFormatBpp(CameraFormat::kRgb)));
  std::vector<tensorflow::Class> results;
  if (ClassifyFromCamera(interpreter, model_width, model_height, bayered,
                         &results, &image)) {
    printf("%s\r\n", tensorflow::FormatClassificationOutput(results).c_str());
  } else {
    printf("Failed to classify image from camera.\r\n");
  }
}

[[noreturn]] void Main() {
  printf("Classify Camera Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  std::vector<uint8_t> model;
  if (!LfsReadFile(kModelPath.c_str(), &model)) {
    printf("ERROR: Failed to load %s\r\n", kModelPath.c_str());
    vTaskSuspend(nullptr);
  }

  auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!tpu_context) {
    printf("ERROR: Failed to get EdgeTpu context\r\n");
    vTaskSuspend(nullptr);
  }

  tflite::MicroErrorReporter error_reporter;
  tflite::MicroMutableOpResolver<1> resolver;
  resolver.AddCustom(kCustomOp, RegisterCustomOp());

  tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()), resolver,
                                       tensor_arena, kTensorArenaSize,
                                       &error_reporter);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("ERROR: AllocateTensors() failed\r\n");
    vTaskSuspend(nullptr);
  }

  if (interpreter.inputs().size() != 1) {
    printf("ERROR: Model must have only one input tensor\r\n");
    vTaskSuspend(nullptr);
  }

  // Starting Camera.
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kTrigger);

  printf("Initializing classification server...\r\n");
  jsonrpc_init(nullptr, &interpreter);
  jsonrpc_export("classify_from_camera", ClassifyRpc);
  UseHttpServer(new JsonRpcHttpServer);
  printf("Classification server ready!\r\n");
  GpioConfigureInterrupt(
      Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
      /*debounce_interval_us=*/50 * 1e3);
  while (true) {
    vTaskSuspend(nullptr);
    ClassifyConsole(&interpreter);
  }
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
