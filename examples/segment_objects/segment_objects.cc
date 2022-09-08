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
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Runs a local RPC server with an endpoint called 'segment_from_camera',
// which will capture an image from the board's camera, run the image through a
// segmentation model and return the results to a connected
// Python client app through an RPC server.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e segment_objects
//
// NOTE: The Python client app works on Windows and Linux only.
//
// After flashing the example, run this Python client to trigger an inference
// with a photo and receive the results over USB:
//    python3 -m pip install -r examples/segment_objects/requirements.txt
//    python3 examples/segment_objects/segment_objects_client.py
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
//     'output_mask': output_mask,
//      }
// }
//
// This can theoretically run any supported segmentation model but has only
// been tested with keras_post_training_unet_mv2_128_quant_edgetpu.tflite
// which comes from the tutorial at
// https://www.tensorflow.org/tutorials/images/segmentation. It is trained on
// the Oxford-IIIT Pet Dataset and will segment into three classes:
// Class 1: Pixel belonging to the pet.
// Class 2: Pixel bordering the pet.
// Class 3: None of the above/a surrounding pixel.

namespace coralmicro {
namespace {

constexpr char kModelPath[] =
    "/models/keras_post_training_unet_mv2_128_quant_edgetpu.tflite";
constexpr int kTensorArenaSize = 2 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

void SegmentFromCamera(struct jsonrpc_request* r) {
  auto* interpreter =
      static_cast<tflite::MicroInterpreter*>(r->ctx->response_cb_data);
  auto* input_tensor = interpreter->input_tensor(0);
  int model_height = input_tensor->dims->data[1];
  int model_width = input_tensor->dims->data[2];

  std::vector<uint8_t> image(model_width * model_height *
                             CameraFormatBpp(CameraFormat::kRgb));
  CameraFrameFormat fmt{CameraFormat::kRgb,   CameraFilterMethod::kBilinear,
                        CameraRotation::k270, model_width,
                        model_height,         false,
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
  const auto& output_tensor = interpreter->output_tensor(0);
  const auto& output_mask = tflite::GetTensorData<uint8_t>(output_tensor);
  const auto mask_size = tensorflow::TensorSize(output_tensor);
  jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V, %Q: %V}", "width",
                         model_width, "height", model_height, "base64_data",
                         image.size(), image.data(), "output_mask", mask_size,
                         output_mask);
}

void Main() {
  printf("Segment Camera Example!\r\n");
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
  tflite::MicroMutableOpResolver<3> resolver;
  resolver.AddResizeBilinear();
  resolver.AddArgMax();
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

  printf("Initializing segmentation server...\r\n");
  jsonrpc_init(nullptr, &interpreter);
  jsonrpc_export("segment_from_camera", SegmentFromCamera);
  UseHttpServer(new JsonRpcHttpServer);
  printf("Segmentation server ready!\r\n");
  vTaskSuspend(nullptr);
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
