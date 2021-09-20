#include "libs/base/utils.h"
#include "libs/posenet/posenet.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testlib/test_lib.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"

#include <array>

void PosenetStressRun(struct jsonrpc_request *request) {
    valiant::posenet::Output output;
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(500));
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);
    bool loopSuccess;
    int iterations;

    if (!valiant::posenet::setup()) {
        printf("setup() failed\r\n");
        valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
        valiant::CameraTask::GetSingleton()->SetPower(false);
        jsonrpc_return_error(request, -1, "Posenet setup() failed", nullptr);
        return;
    }
    valiant::posenet::loop(&output);
    printf("Posenet static datatest finished.\r\n");

    uint8_t *buffer = nullptr;
    int index = -1;
    for (int i = 0; i < 100; ++i) {
        index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
        valiant::CameraTask::GetSingleton()->ReturnFrame(index);
    }
    if(!valiant::testlib::JSONRPCGetIntegerParam(request, "iterations", &iterations)){
            valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
            valiant::CameraTask::GetSingleton()->SetPower(false);
            jsonrpc_return_error(request, -1, "Failed to get int iterations.", nullptr);
            return;
    }
    for (int i = 0; i < iterations; i++) {
        TfLiteTensor *input = valiant::posenet::input();
        valiant::camera::FrameFormat fmt;
        fmt.width = input->dims->data[2];
        fmt.height = input->dims->data[1];
        fmt.fmt = valiant::camera::Format::RGB;
        fmt.preserve_ratio = false;
        fmt.buffer = tflite::GetTensorData<uint8_t>(input);
        valiant::CameraTask::GetFrame({fmt});
        loopSuccess = valiant::posenet::loop(&output);
        if(!loopSuccess){
            valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
            valiant::CameraTask::GetSingleton()->SetPower(false);
            jsonrpc_return_error(request, -1, "Posenet loop() returned failure", nullptr);
            return;
        }

    }
    valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
    valiant::CameraTask::GetSingleton()->SetPower(false);
    jsonrpc_return_success(request, "{}");
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
    rpc_server.RegisterRPC("get_serial_number",
                           valiant::testlib::GetSerialNumber);
    rpc_server.RegisterRPC("run_testconv1",
                           valiant::testlib::RunTestConv1);
    rpc_server.RegisterRPC("set_tpu_power_state",
                           valiant::testlib::SetTPUPowerState);
    rpc_server.RegisterRPC("posenet_stress_run", PosenetStressRun);
    rpc_server.RegisterRPC(valiant::testlib::kBeginUploadResourceName,
                           valiant::testlib::BeginUploadResource);
    rpc_server.RegisterRPC(valiant::testlib::kUploadResourceChunkName,
                           valiant::testlib::UploadResourceChunk);
    rpc_server.RegisterRPC(valiant::testlib::kDeleteResourceName,
                           valiant::testlib::DeleteResource);
    rpc_server.RegisterRPC(valiant::testlib::kRunClassificationModelName,
                           valiant::testlib::RunClassificationModel);
    vTaskSuspend(NULL);
}
