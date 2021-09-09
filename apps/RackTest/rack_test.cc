#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/base/utils.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <array>

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

    vTaskSuspend(NULL);
}
