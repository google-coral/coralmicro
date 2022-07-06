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

#include <cstdio>

#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"

#if defined(IMAGE_SERVER_ETHERNET)
#include "libs/base/ethernet.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"
#endif  // defined(IMAGE_SERVER_ETHERNET)

#if defined(IMAGE_SERVER_WIFI)
#include "libs/base/wifi.h"
#endif  // defined(IMAGE_SERVER_WIFI)

namespace {
using coralmicro::testlib::JsonRpcGetIntegerParam;

void get_image_from_camera(struct jsonrpc_request* request) {
    int width, height;
    if (!JsonRpcGetIntegerParam(request, "width", &width)) {
        return;
    }
    if (!JsonRpcGetIntegerParam(request, "height", &height)) {
        return;
    }

    coralmicro::CameraTask::GetSingleton()->SetPower(true);
    coralmicro::CameraTask::GetSingleton()->Enable(
        coralmicro::camera::Mode::kStreaming);
    std::vector<uint8_t> image(width * height * /*channels=*/3);
    coralmicro::camera::FrameFormat fmt{
        coralmicro::camera::Format::kRgb,
        coralmicro::camera::FilterMethod::kBilinear,
        coralmicro::camera::Rotation::k0,
        width,
        height,
        false,
        image.data()};
    auto ret = coralmicro::CameraTask::GetFrame({fmt});
    coralmicro::CameraTask::GetSingleton()->Disable();
    coralmicro::CameraTask::GetSingleton()->SetPower(false);

    if (!ret) {
        jsonrpc_return_error(request, -1, "Failed to get image from camera.",
                             nullptr);
        return;
    }
    jsonrpc_return_success(request, "{%Q: %d, %Q: %d, %Q: %V}", "width", width,
                           "height", height, "base64_data", image.size(),
                           image.data());
}

}  // namespace

extern "C" void app_main(void* param) {
#if defined(IMAGE_SERVER_ETHERNET)
    coralmicro::InitializeEthernet(true);
    auto* ethernet = coralmicro::GetEthernetInterface();
    if (!ethernet) {
        printf("Unable to bring up ethernet...\r\n");
        vTaskSuspend(nullptr);
    }
    auto ethernet_ip = coralmicro::GetEthernetIp();
    if (!ethernet_ip.has_value()) {
        printf("Unable to get Ethernet IP\r\n");
        vTaskSuspend(nullptr);
    }
    printf("Starting Image RPC Server on: %s\r\n", ethernet_ip.value().c_str());
    jsonrpc_init(nullptr, &ethernet_ip.value());
    jsonrpc_export("get_ethernet_ip", [](struct jsonrpc_request* request) {
        jsonrpc_return_success(
            request, "{%Q: %Q}", "ethernet_ip",
            reinterpret_cast<std::string*>(request->ctx->response_cb_data)->c_str());
    });
#elif defined(IMAGE_SERVER_WIFI)
    if (!coralmicro::TurnOnWiFi()) {
        printf("Unable to bring up wifi...\r\n");
    }
    jsonrpc_export(coralmicro::testlib::kMethodWiFiConnect,
                   coralmicro::testlib::WiFiConnect);
    jsonrpc_export(coralmicro::testlib::kMethodWiFiGetIp,
                   coralmicro::testlib::WiFiGetIp);
    jsonrpc_export(coralmicro::testlib::kMethodWiFiGetStatus,
                   coralmicro::testlib::WiFiGetStatus);

#else
    printf("Starting Image RPC Server...\r\n");
    jsonrpc_init(nullptr, nullptr);
#endif  // defined(IMAGE_SERVER_ETHERNET)
    jsonrpc_export("get_image_from_camera", get_image_from_camera);
    coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
    printf("Server started...\r\n");
    vTaskSuspend(nullptr);
}
