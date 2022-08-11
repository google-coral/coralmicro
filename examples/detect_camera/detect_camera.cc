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
#include "libs/tensorflow/detection.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Runs a local server with an endpoint called 'detect_from_camera', which
// will capture an image from the board's camera, run the image through an
// object detection model and return the results in a JSON response.
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
//     'detection':
//         {
//         'id': int,
//         'score': float,
//         'xmin': float,
//         'xmax': float,
//         'ymin': float,
//         'ymax': float
//         }
//     }
// }

namespace coralmicro {
namespace {
constexpr char kModelPath[] =
    "/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite";
// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

void DetectFromCamera(struct jsonrpc_request* r) {
  auto* interpreter =
      reinterpret_cast<tflite::MicroInterpreter*>(r->ctx->response_cb_data);

  auto* input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];

  printf("width=%d; height=%d\r\n", model_width, model_height);

  std::vector<uint8_t> image(model_width * model_height * /*channels=*/3);
  CameraFrameFormat fmt{CameraFormat::kRgb, CameraFilterMethod::kBilinear,
                        CameraRotation::k0, model_width,
                        model_height,       false,
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

  auto results = tensorflow::GetDetectionResults(interpreter, 0.5, 1);
  if (!results.empty()) {
    const auto& result = results[0];
    jsonrpc_return_success(
        r,
        "{%Q: %d, %Q: %d, %Q: %V, %Q: {%Q: %d, %Q: %g, %Q: %g, %Q: %g, "
        "%Q: %g, %Q: %g}}",
        "width", model_width, "height", model_height, "base64_data",
        image.size(), image.data(), "detection", "id", result.id, "score",
        result.score, "xmin", result.bbox.xmin, "xmax", result.bbox.xmax,
        "ymin", result.bbox.ymin, "ymax", result.bbox.ymax);
    return;
  }
  jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: None}", "width",
                         model_width, "height", model_height, "base64_data",
                         image.size(), image.data(), "detection");
}

void Main() {
  printf("Detection Camera Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  std::vector<uint8_t> model;
  if (!LfsReadFile(kModelPath, &model)) {
    printf("ERROR: Failed to load %s\r\n", kModelPath);
    vTaskSuspend(nullptr);
  }

  auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
  if (!tpu_context) {
    printf("ERROR: Failed to get EdgeTpu context\r\n");
    vTaskSuspend(nullptr);
  }

  tflite::MicroErrorReporter error_reporter;
  tflite::MicroMutableOpResolver<3> resolver;
  resolver.AddDequantize();
  resolver.AddDetectionPostprocess();
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

  printf("Initializing detection server...%p\r\n", &interpreter);
  jsonrpc_init(nullptr, &interpreter);
  jsonrpc_export("detect_from_camera", DetectFromCamera);
  UseHttpServer(new JsonRpcHttpServer);
  printf("Detection server ready!\r\n");
  vTaskSuspend(nullptr);
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
