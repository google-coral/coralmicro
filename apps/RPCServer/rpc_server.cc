#include "libs/base/utils.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"

#include <cassert>
#include <cstdio>
#include <map>
#include <memory>
#include <vector>

static void serial_number_rpc(struct jsonrpc_request *r) {
    std::string serial = valiant::utils::GetSerialNumber();
    jsonrpc_return_success(r, "{%Q:%.*Q}", "serial_number", serial.size(),
                           serial.c_str());
}

static void take_picture_rpc(struct jsonrpc_request *r) {
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
    valiant::CameraTask::GetSingleton()->DiscardFrames(1);

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
        jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V}",
            "width", valiant::CameraTask::kWidth,
            "height", valiant::CameraTask::kHeight,
            "pixels", image_buffer.size(), image_buffer.data());
    } else {
        jsonrpc_return_error(r, -1, "failure", nullptr);
    }
}

extern "C" void app_main(void *param) {
    jsonrpc_init(nullptr, nullptr);
    jsonrpc_export("serial_number", serial_number_rpc);
    jsonrpc_export("take_picture", take_picture_rpc);
    valiant::rpc::RpcServer rpc_server;
    if (!rpc_server.Init(&jsonrpc_default_context)) {
        printf("Failed to initialize RPCServerIOHTTP\r\n");
        vTaskSuspend(NULL);
    }

    printf("RPC server ready\r\n");
    vTaskSuspend(NULL);
}
