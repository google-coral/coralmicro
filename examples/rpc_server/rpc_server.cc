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

#include <cassert>
#include <cstdio>
#include <map>
#include <memory>
#include <vector>

#include "libs/base/utils.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"

// Runs a local server with two RPC endpoints: 'serial_number', which
// returns the board's SN, and 'take_picture', which captures an image with
// the board's camera and return it via JSON.
//
// The 'take_picture' response looks like this:
//
// {
// 'id': int,
// 'result':
//     {
//     'width': int,
//     'height': int,
//     'base64_data': image_bytes
//     }
// }

static void serial_number_rpc(struct jsonrpc_request* r) {
    std::string serial = coralmicro::utils::GetSerialNumber();
    jsonrpc_return_success(r, "{%Q:%.*Q}", "serial_number", serial.size(),
                           serial.c_str());
}

static void take_picture_rpc(struct jsonrpc_request* r) {
    coralmicro::CameraTask::GetSingleton()->SetPower(true);
    coralmicro::CameraTask::GetSingleton()->Enable(
        coralmicro::camera::Mode::kStreaming);
    coralmicro::CameraTask::GetSingleton()->DiscardFrames(1);

    std::vector<uint8_t> image_buffer(coralmicro::CameraTask::kWidth *
                                      coralmicro::CameraTask::kHeight * 3);
    coralmicro::camera::FrameFormat fmt;
    fmt.fmt = coralmicro::camera::Format::kRgb;
    fmt.filter = coralmicro::camera::FilterMethod::kBilinear;
    fmt.width = coralmicro::CameraTask::kWidth;
    fmt.height = coralmicro::CameraTask::kHeight;
    fmt.preserve_ratio = false;
    fmt.buffer = image_buffer.data();

    bool ret = coralmicro::CameraTask::GetFrame({fmt});

    coralmicro::CameraTask::GetSingleton()->Disable();
    coralmicro::CameraTask::GetSingleton()->SetPower(false);
    if (ret) {
        jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V}", "width",
                               coralmicro::CameraTask::kWidth, "height",
                               coralmicro::CameraTask::kHeight, "base64_data",
                               image_buffer.size(), image_buffer.data());
    } else {
        jsonrpc_return_error(r, -1, "failure", nullptr);
    }
}

extern "C" void app_main(void* param) {
    jsonrpc_init(nullptr, nullptr);
    jsonrpc_export("serial_number", serial_number_rpc);
    jsonrpc_export("take_picture", take_picture_rpc);
    coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
    printf("RPC server ready\r\n");
    vTaskSuspend(nullptr);
}
