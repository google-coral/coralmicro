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
// object detection model on the Edge TPU and return the results to a connected
// Python client app through an RPC server.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e detect_objects
//
// NOTE: The Python client app works on Windows and Linux only.
//
// After flashing the example, run this Python client to trigger an inference
// with a photo and receive the results over USB:
//    python3 -m pip install -r examples/detect_objects/requirements.txt
//    python3 examples/detect_objects/detect_objects_client.py
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

bool DetectFromCamera(tflite::MicroInterpreter* interpreter, int model_width,
                      int model_height,
                      std::vector<tensorflow::Object>* results,
                      std::vector<uint8>* image) {
  CHECK(results != nullptr);
  CHECK(image != nullptr);
  auto* input_tensor = interpreter->input_tensor(0);
  CameraFrameFormat fmt{CameraFormat::kRgb,   CameraFilterMethod::kBilinear,
                        CameraRotation::k270, model_width,
                        model_height,         false,
                        image->data()};

  CameraTask::GetSingleton()->Trigger();
  if (!CameraTask::GetSingleton()->GetFrame({fmt})) return false;

  std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image->data(),
              image->size());
  if (interpreter->Invoke() != kTfLiteOk) return false;

  *results = tensorflow::GetDetectionResults(interpreter, 0.5, 1);
  return true;
}

void DetectRpc(struct jsonrpc_request* r) {
  auto* interpreter =
      static_cast<tflite::MicroInterpreter*>(r->ctx->response_cb_data);
  auto* input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];
  std::vector<uint8> image(model_height * model_width *
                           CameraFormatBpp(CameraFormat::kRgb));
  std::vector<tensorflow::Object> results;
  if (DetectFromCamera(interpreter, model_width, model_height, &results,
                       &image)) {
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
    return;
  }
  jsonrpc_return_error(r, -1, "Failed to detect image from camera.", nullptr);
}

void DetectConsole(tflite::MicroInterpreter* interpreter) {
  auto* input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];
  std::vector<uint8> image(model_height * model_width *
                           CameraFormatBpp(CameraFormat::kRgb));
  std::vector<tensorflow::Object> results;
  if (DetectFromCamera(interpreter, model_width, model_height, &results,
                       &image)) {
    printf("%s\r\n", tensorflow::FormatDetectionOutput(results).c_str());
  } else {
    printf("Failed to detect image from camera.\r\n");
  }
}

[[noreturn]] void Main() {
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

  printf("Initializing detection server...\r\n");
  jsonrpc_init(nullptr, &interpreter);
  jsonrpc_export("detect_from_camera", DetectRpc);
  UseHttpServer(new JsonRpcHttpServer);
  printf("Detection server ready!\r\n");
  GpioConfigureInterrupt(
      Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
      /*debounce_interval_us=*/50 * 1e3);
  while (true) {
    vTaskSuspend(nullptr);
    DetectConsole(&interpreter);
  }
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
