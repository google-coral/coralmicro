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
using coralmicro::testlib::JsonRpcGetBooleanParam;
using coralmicro::testlib::JsonRpcGetIntegerParam;
using coralmicro::testlib::JsonRpcGetStringParam;

bool FormatStringToFormat(const std::string& format_string, coralmicro::camera::Format* format) {
    if (format_string == "RGB") {
        *format = coralmicro::camera::Format::kRgb;
        return true;
    }
    if (format_string == "GRAY") {
        *format = coralmicro::camera::Format::kY8;
        return true;
    }
    if (format_string == "RAW") {
        *format = coralmicro::camera::Format::kRaw;
        return true;
    }
    return false;
}

bool FilterStringToFilter(const std::string& filter_string, coralmicro::camera::FilterMethod* filter) {
    if (filter_string == "BILINEAR") {
        *filter = coralmicro::camera::FilterMethod::kBilinear;
        return true;
    }
    if (filter_string == "NEAREST_NEIGHBOR") {
        *filter = coralmicro::camera::FilterMethod::kNearestNeighbor;
        return true;
    }
    return false;
}

bool RotationIntToRotation(const int& rotation_int, coralmicro::camera::Rotation* rotation) {
    switch (rotation_int) {
        case 0:
            *rotation = coralmicro::camera::Rotation::k0;
            return true;
        case 90:
            *rotation = coralmicro::camera::Rotation::k90;
            return true;
        case 180:
            *rotation = coralmicro::camera::Rotation::k180;
            return true;
        case 270:
            *rotation = coralmicro::camera::Rotation::k270;
            return true;
        default:
            return false;
    }
}

void get_image_from_camera(struct jsonrpc_request* request) {
    int width = coralmicro::CameraTask::kWidth;
    int height = coralmicro::CameraTask::kHeight;
    coralmicro::camera::Format format = coralmicro::camera::Format::kRgb;
    coralmicro::camera::FilterMethod filter = coralmicro::camera::FilterMethod::kBilinear;
    coralmicro::camera::Rotation rotation = coralmicro::camera::Rotation::k0;
    bool auto_white_balance = true;

    std::string format_rpc, filter_rpc;
    int rotation_rpc;
    bool awb_rpc;
    if (!JsonRpcGetIntegerParam(request, "width", &width)) {
        return;
    }
    if (!JsonRpcGetIntegerParam(request, "height", &height)) {
        return;
    }
    if (!JsonRpcGetStringParam(request, "format", &format_rpc)) {
        return;
    }
    if (!JsonRpcGetStringParam(request, "filter", &filter_rpc)) {
        return;
    }
    if (!JsonRpcGetIntegerParam(request, "rotation", &rotation_rpc)) {
        return;
    }
    if (!JsonRpcGetBooleanParam(request, "auto_white_balance", &awb_rpc)) {
        return;
    }

    if (!FormatStringToFormat(format_rpc, &format)) {
        jsonrpc_return_error(request, -1, "Unknown 'format'", nullptr);
        return;
    }
    if (!FilterStringToFilter(filter_rpc, &filter)) {
        jsonrpc_return_error(request, -1, "Unknown 'filter'", nullptr);
        return;
    }
    if (!RotationIntToRotation(rotation_rpc, &rotation)) {
        jsonrpc_return_error(request, -1, "Unknown 'rotation'", nullptr);
        return;
    }

    coralmicro::CameraTask::GetSingleton()->SetPower(true);
    coralmicro::CameraTask::GetSingleton()->Enable(
        coralmicro::camera::Mode::kStreaming);
    std::vector<uint8_t> image(width * height * coralmicro::CameraTask::FormatToBPP(format));
    coralmicro::camera::FrameFormat fmt{
        format,
        filter,
        rotation,
        width,
        height,
        false,
        image.data(),
        auto_white_balance};
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
    if (!coralmicro::WiFiTurnOn()) {
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
