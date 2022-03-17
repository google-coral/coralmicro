#include "libs/base/ethernet.h"
#include "libs/base/main_freertos_m7.h"
#include "libs/testlib/test_lib.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"

static void EthGetIP(struct jsonrpc_request *request) {
    struct netif* ethernet = valiant::GetEthernetInterface();
    if (!ethernet) {
        jsonrpc_return_error(request, -1, "ethernet interface not found", nullptr);
        return;
    }

    if (!netif_is_up(ethernet)) {
        jsonrpc_return_error(request, -1, "ethernet interface is not up", nullptr);
        return;
    }

    struct dhcp *dhcp = netif_dhcp_data(ethernet);
    if (dhcp->state != DHCP_STATE_BOUND) {
        jsonrpc_return_error(request, -1, "dhcp not complete", nullptr);
        return;
    }

    jsonrpc_return_success(request, "{%Q:%Q}", "ip", ip4addr_ntoa(netif_ip4_addr(ethernet)));
}

static void EthWritePHY(struct jsonrpc_request *request) {
    int reg;
    if (!valiant::testlib::JsonRpcGetIntegerParam(request, "reg", &reg)) return;

    int val;
    if (!valiant::testlib::JsonRpcGetIntegerParam(request, "val", &val)) return;

    status_t status = valiant::EthernetPHYWrite(reg, val);
    if (status != kStatus_Success) {
        jsonrpc_return_error(request, -1, "EthernetPHYWrite failed", nullptr);
        return;
    }

    jsonrpc_return_success(request, "{}");
}

extern "C" void app_main(void *param) {
    valiant::InitializeEthernet(false);
    valiant::rpc::RPCServerIOHTTP rpc_server_io_http;
    if (!rpc_server_io_http.Init()) {
        printf("Failed to initialize RPCServerIOHTTP\r\n");
        vTaskSuspend(NULL);
    }

    jsonrpc_init(nullptr, nullptr);
    jsonrpc_export("eth_get_ip", EthGetIP);
    jsonrpc_export("eth_write_phy", EthWritePHY);
    rpc_server_io_http.SetContext(&jsonrpc_default_context);

    vTaskSuspend(NULL);
}

extern "C" int main(int argc, char **argv) {
    return real_main(argc, argv, false, false);
}
