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

namespace {
constexpr char kModelPath[] = "/models/mnv2_324_quant_bayered_edgetpu.tflite";
constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

void ClassifyFromCamera(struct jsonrpc_request* r) {
    auto* interpreter =
        reinterpret_cast<tflite::MicroInterpreter*>(r->ctx->response_cb_data);

    auto* input_tensor = interpreter->input_tensor(0);
    int model_height = input_tensor->dims->data[1];
    int model_width = input_tensor->dims->data[2];

    coralmicro::CameraTask::GetSingleton()->SetPower(true);
    coralmicro::CameraTask::GetSingleton()->Enable(
        coralmicro::camera::Mode::kStreaming);

    // If the model name includes "bayered", provide the raw datastream from the
    // camera.
    bool bayered = strstr(kModelPath, "bayered");
    std::vector<uint8_t> image(model_width * model_height *
                               /*channels=*/(bayered ? 1 : 3));
    auto format = bayered ? coralmicro::camera::Format::kRaw
                          : coralmicro::camera::Format::kRgb;
    coralmicro::camera::FrameFormat fmt{
        format,
        coralmicro::camera::FilterMethod::kBilinear,
        coralmicro::camera::Rotation::k0,
        model_width,
        model_height,
        false,
        image.data()};

    // Discard the first frame to ensure no power-on artifacts exist.
    bool ret = coralmicro::CameraTask::GetSingleton()->GetFrame({fmt});
    ret = coralmicro::CameraTask::GetSingleton()->GetFrame({fmt});

    coralmicro::CameraTask::GetSingleton()->Disable();
    coralmicro::CameraTask::GetSingleton()->SetPower(false);

    if (!ret) {
        jsonrpc_return_error(r, -1, "Failed to get image from camera.",
                             nullptr);
        return;
    }

    std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image.data(),
                image.size());

    if (interpreter->Invoke() != kTfLiteOk) {
        jsonrpc_return_error(r, -1, "Invoke failed", nullptr);
        return;
    }

    auto results =
        coralmicro::tensorflow::GetClassificationResults(interpreter, 0.0f, 1);
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
}
}  // namespace

extern "C" void app_main(void* param) {
    std::vector<uint8_t> model;
    if (!coralmicro::LfsReadFile(kModelPath, &model)) {
        printf("ERROR: Failed to load %s\r\n", kModelPath);
        vTaskSuspend(nullptr);
    }

    auto tpu_context = coralmicro::EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!tpu_context) {
        printf("ERROR: Failed to get EdgeTpu context\r\n");
        vTaskSuspend(nullptr);
    }

    tflite::MicroErrorReporter error_reporter;
    tflite::MicroMutableOpResolver<1> resolver;
    resolver.AddCustom(coralmicro::kCustomOp, coralmicro::RegisterCustomOp());

    tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()),
                                         resolver, tensor_arena,
                                         kTensorArenaSize, &error_reporter);
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        printf("ERROR: AllocateTensors() failed\r\n");
        vTaskSuspend(nullptr);
    }

    if (interpreter.inputs().size() != 1) {
        printf("ERROR: Model must have only one input tensor\r\n");
        vTaskSuspend(nullptr);
    }

    printf("Initializing classification server...%p\r\n", &interpreter);
    jsonrpc_init(nullptr, &interpreter);
    jsonrpc_export("classify_from_camera", ClassifyFromCamera);
    coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
    printf("Classification server ready!\r\n");
    vTaskSuspend(nullptr);
}
