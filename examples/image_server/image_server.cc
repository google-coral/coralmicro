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
using coral::micro::testlib::JsonRpcGetIntegerParam;

#if defined(IMAGE_SERVER_ETHERNET)
char* get_ethernet_ip(struct netif* ethernet) {
    while (true) {
        auto* dhcp = netif_dhcp_data(ethernet);
        if (dhcp->state == DHCP_STATE_BOUND) {
            break;
        }
        taskYIELD();
    }
    return ip4addr_ntoa(netif_ip4_addr(ethernet));
}
#endif  // defined(IMAGE_SERVER_ETHERNET)

void get_image_from_camera(struct jsonrpc_request* request) {
    int width, height;
    if (!JsonRpcGetIntegerParam(request, "width", &width)) {
        return;
    }
    if (!JsonRpcGetIntegerParam(request, "height", &height)) {
        return;
    }

    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::kStreaming);
    std::vector<uint8_t> image(width * height * /*channels=*/3);
    coral::micro::camera::FrameFormat fmt{
        coral::micro::camera::Format::kRgb,
        coral::micro::camera::FilterMethod::kBilinear,
        coral::micro::camera::Rotation::k0,
        width,
        height,
        false,
        image.data()};
    auto ret = coral::micro::CameraTask::GetFrame({fmt});
    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::CameraTask::GetSingleton()->SetPower(false);

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
    coral::micro::InitializeEthernet(true);
    auto* ethernet = coral::micro::GetEthernetInterface();
    if (!ethernet) {
        printf("Unable to bring up ethernet...\r\n");
        vTaskSuspend(nullptr);
    }
    auto* ethernet_ip = get_ethernet_ip(ethernet);
    printf("Starting Image RPC Server on: %s\r\n", ethernet_ip);
    jsonrpc_init(nullptr, ethernet_ip);
    jsonrpc_export("get_ethernet_ip", [](struct jsonrpc_request* request) {
        jsonrpc_return_success(
            request, "{%Q: %Q}", "ethernet_ip",
            reinterpret_cast<char*>(request->ctx->response_cb_data));
    });
#elif defined(IMAGE_SERVER_WIFI)
    if (!coral::micro::TurnOnWiFi()) {
        printf("Unable to bring up wifi...\r\n");
    }
    jsonrpc_export(coral::micro::testlib::kMethodWiFiConnect,
                   coral::micro::testlib::WiFiConnect);
    jsonrpc_export(coral::micro::testlib::kMethodWiFiGetIp,
                   coral::micro::testlib::WiFiGetIp);
    jsonrpc_export(coral::micro::testlib::kMethodWiFiGetStatus,
                   coral::micro::testlib::WiFiGetStatus);

#else
    printf("Starting Image RPC Server...\r\n");
    jsonrpc_init(nullptr, nullptr);
#endif  // defined(IMAGE_SERVER_ETHERNET)
    jsonrpc_export("get_image_from_camera", get_image_from_camera);
    coral::micro::UseHttpServer(new coral::micro::JsonRpcHttpServer);
    printf("Server started...\r\n");
    vTaskSuspend(nullptr);
}
