#include <cstdio>

#include "libs/rpc/rpc_http_server.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"

#if defined(IMAGE_SERVER_ETHERNET)
#include "libs/base/ethernet.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"
#endif  // defined(IMAGE_SERVER_ETHERNET)

namespace {
using valiant::testlib::JsonRpcGetIntegerParam;

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

    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(
        valiant::camera::Mode::STREAMING);
    std::vector<uint8_t> image(width * height * /*channels=*/3);
    valiant::camera::FrameFormat fmt{valiant::camera::Format::RGB,
                                     valiant::camera::FilterMethod::BILINEAR,
                                     width, height, false, image.data()};
    auto ret = valiant::CameraTask::GetFrame({fmt});
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);

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
    valiant::InitializeEthernet(true);
    auto* ethernet = valiant::GetEthernetInterface();
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
#else
    printf("Starting Image RPC Server...\r\n");
    jsonrpc_init(nullptr, nullptr);
#endif  // defined(IMAGE_SERVER_ETHERNET)
    jsonrpc_export("get_image_from_camera", get_image_from_camera);
    valiant::UseHttpServer(new valiant::JsonRpcHttpServer);
    printf("Server started...\r\n");
    vTaskSuspend(nullptr);
}
