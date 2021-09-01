#include "libs/base/ethernet.h"
#include "libs/base/main_freertos_m7.h"
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

extern "C" void app_main(void *param) {
    valiant::InitializeEthernet(false);

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

    rpc_server.RegisterRPC("eth_get_ip", EthGetIP);

    vTaskSuspend(NULL);
}

extern "C" int main(int argc, char **argv) {
    return real_main(argc, argv, false, false);
}
