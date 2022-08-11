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
// which will capture an image from the board's camera, run the image through a
// classification model and return the results in a JSON response.
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
constexpr char kModelPath[] = "/models/mnv2_324_quant_bayered_edgetpu.tflite";
constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

void ClassifyFromCamera(struct jsonrpc_request* r) {
  auto* interpreter =
      static_cast<tflite::MicroInterpreter*>(r->ctx->response_cb_data);

  auto* input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];

  // If the model name includes "bayered", provide the raw datastream from the
  // camera.
  bool bayered = strstr(kModelPath, "bayered");
  std::vector<uint8_t> image(model_width * model_height *
                             /*channels=*/(bayered ? 1 : 3));
  auto format = bayered ? CameraFormat::kRaw : CameraFormat::kRgb;
  CameraFrameFormat fmt{format,
                        CameraFilterMethod::kBilinear,
                        CameraRotation::k270,
                        model_width,
                        model_height,
                        false,
                        image.data()};

  CameraTask::GetSingleton()->Trigger();
  bool ret = CameraTask::GetSingleton()->GetFrame({fmt});

  if (!ret) {
    jsonrpc_return_error(r, -1, "Failed to get image from camera.", nullptr);
    return;
  }

  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image.data(),
              image.size());

  if (interpreter->Invoke() != kTfLiteOk) {
    jsonrpc_return_error(r, -1, "Invoke failed", nullptr);
    return;
  }

  auto results = tensorflow::GetClassificationResults(interpreter, 0.0f, 1);
  if (!results.empty()) {
    const auto& result = results[0];
    jsonrpc_return_success(r,
                           "{%Q: %d, %Q: %d, %Q: %V, %Q: %d, %Q: %d, %Q: %g}",
                           "width", model_width, "height", model_height,
                           "base64_data", image.size(), image.data(), "bayered",
                           bayered, "id", result.id, "score", result.score);
    return;
  }
  jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: %d}", "width",
                         model_width, "height", model_height, "base64_data",
                         image.size(), image.data(), "bayered", bayered);
}

void Main() {
  printf("Classify Camera Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  std::vector<uint8_t> model;
  if (!LfsReadFile(kModelPath, &model)) {
    printf("ERROR: Failed to load %s\r\n", kModelPath);
    return;
  }

  auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!tpu_context) {
    printf("ERROR: Failed to get EdgeTpu context\r\n");
    return;
  }

  tflite::MicroErrorReporter error_reporter;
  tflite::MicroMutableOpResolver<1> resolver;
  resolver.AddCustom(kCustomOp, RegisterCustomOp());

  tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()), resolver,
                                       tensor_arena, kTensorArenaSize,
                                       &error_reporter);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("ERROR: AllocateTensors() failed\r\n");
    return;
  }

  if (interpreter.inputs().size() != 1) {
    printf("ERROR: Model must have only one input tensor\r\n");
    return;
  }

  // Starting Camera.
  CameraTask::GetSingleton()->SetPower(true);
  CameraTask::GetSingleton()->Enable(CameraMode::kTrigger);

  printf("Initializing classification server...%p\r\n", &interpreter);
  jsonrpc_init(nullptr, &interpreter);
  jsonrpc_export("classify_from_camera", ClassifyFromCamera);
  UseHttpServer(new JsonRpcHttpServer);
  printf("Classification server ready!\r\n");
  vTaskSuspend(nullptr);
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}