#include <cassert>
#include <cstdio>
#include <map>
#include <memory>
#include <vector>

#include "libs/base/utils.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"

static void serial_number_rpc(struct jsonrpc_request* r) {
    std::string serial = coral::micro::utils::GetSerialNumber();
    jsonrpc_return_success(r, "{%Q:%.*Q}", "serial_number", serial.size(),
                           serial.c_str());
}

static void take_picture_rpc(struct jsonrpc_request* r) {
    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::STREAMING);
    coral::micro::CameraTask::GetSingleton()->DiscardFrames(1);

    std::vector<uint8_t> image_buffer(coral::micro::CameraTask::kWidth *
                                      coral::micro::CameraTask::kHeight * 3);
    coral::micro::camera::FrameFormat fmt;
    fmt.fmt = coral::micro::camera::Format::RGB;
    fmt.filter = coral::micro::camera::FilterMethod::BILINEAR;
    fmt.width = coral::micro::CameraTask::kWidth;
    fmt.height = coral::micro::CameraTask::kHeight;
    fmt.preserve_ratio = false;
    fmt.buffer = image_buffer.data();

    bool ret = coral::micro::CameraTask::GetFrame({fmt});

    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    if (ret) {
        jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %V}", "width",
                               coral::micro::CameraTask::kWidth, "height",
                               coral::micro::CameraTask::kHeight, "pixels",
                               image_buffer.size(), image_buffer.data());
    } else {
        jsonrpc_return_error(r, -1, "failure", nullptr);
    }
}

extern "C" void app_main(void* param) {
    jsonrpc_init(nullptr, nullptr);
    jsonrpc_export("serial_number", serial_number_rpc);
    jsonrpc_export("take_picture", take_picture_rpc);
    coral::micro::UseHttpServer(new coral::micro::JsonRpcHttpServer);
    printf("RPC server ready\r\n");
    vTaskSuspend(NULL);
}
