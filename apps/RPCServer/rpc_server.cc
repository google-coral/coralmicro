#include "libs/base/utils.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/httpsrv/httpsrv_base64.h"

#include <cassert>
#include <cstdio>
#include <map>
#include <memory>
#include <vector>

static void serial_number_rpc(struct jsonrpc_request *r) {
    char serial_number_str[16];
    uint64_t serial_number = valiant::utils::GetUniqueID();
    for (int i = 0; i < 16; ++i) {
        uint8_t nibble = (serial_number >> (i * 4)) & 0xF;
        if (nibble < 10) {
            serial_number_str[15 - i] = nibble + '0';
        } else {
            serial_number_str[15 - i] = (nibble - 10) + 'a';
        }
    }
    jsonrpc_return_success(r, "{%Q:%.*Q}", "serial_number", 16, serial_number_str);
}

static void take_picture_rpc(struct jsonrpc_request *r) {
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
    valiant::CameraTask::GetSingleton()->DiscardFrames(100);

    std::vector<uint8_t> image_buffer(
        valiant::CameraTask::kWidth *
        valiant::CameraTask::kHeight *
        3
    );
    valiant::camera::FrameFormat fmt;
    fmt.fmt = valiant::camera::Format::RGB;
    fmt.width = valiant::CameraTask::kWidth;
    fmt.height = valiant::CameraTask::kHeight;
    fmt.preserve_ratio = false;
    fmt.buffer = image_buffer.data();

    bool ret = valiant::CameraTask::GetFrame({fmt});

    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);
    if (ret) {
        size_t base64_size = (((image_buffer.size() * 4) / 3) + 4 - 1) & -4;
        std::vector<char> base64_data(base64_size);
        base64_encode_binary(reinterpret_cast<char*>(image_buffer.data()), base64_data.data(), image_buffer.size());
        jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %d, %Q: %d, %Q: %.*Q}",
            "width", valiant::CameraTask::kWidth,
            "height", valiant::CameraTask::kHeight,
            "image_data_size", image_buffer.size(),
            "base64_size", base64_size,
            "base64_data", base64_data.size(), base64_data.data()
        );
    } else {
        jsonrpc_return_error(r, -1, "failure", nullptr);
    }
}

extern "C" void app_main(void *param) {
    valiant::rpc::RPCServerIOHTTP rpc_server_io_http;
    valiant::rpc::RPCServer rpc_server;
    if (!rpc_server_io_http.Init()) {
        printf("Failed to initialize RPCServerIOHTTP\r\n");
        vTaskSuspend(NULL);
    }
    if (!rpc_server.Init()) {
        printf("Failed to initialize RPCServer\r\n");
        vTaskSuspend(NULL);
    }
    rpc_server.RegisterIO(rpc_server_io_http);

    rpc_server.RegisterRPC("serial_number", serial_number_rpc);
    rpc_server.RegisterRPC("take_picture", take_picture_rpc);

    printf("RPC server ready\r\n");

    vTaskSuspend(NULL);
}
