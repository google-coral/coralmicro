#include "apps/MFGTest/mfg_test_iperf.h"
#include "libs/base/ethernet.h"
#include "libs/base/main_freertos_m7.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"

namespace {
void EthGetIP(struct jsonrpc_request* request) {
  struct netif* ethernet = coral::micro::GetEthernetInterface();
  if (!ethernet) {
    jsonrpc_return_error(request, -1, "ethernet interface not found", nullptr);
    return;
  }

  if (!netif_is_up(ethernet)) {
    jsonrpc_return_error(request, -1, "ethernet interface is not up", nullptr);
    return;
  }

  struct dhcp* dhcp = netif_dhcp_data(ethernet);
  if (dhcp->state != DHCP_STATE_BOUND) {
    jsonrpc_return_error(request, -1, "dhcp not complete", nullptr);
    return;
  }

  jsonrpc_return_success(request, "{%Q:%Q}", "ip",
                         ip4addr_ntoa(netif_ip4_addr(ethernet)));
}

void EthWritePHY(struct jsonrpc_request* request) {
  int reg;
  if (!coral::micro::testlib::JsonRpcGetIntegerParam(request, "reg", &reg))
    return;

  int val;
  if (!coral::micro::testlib::JsonRpcGetIntegerParam(request, "val", &val))
    return;

  status_t status = coral::micro::EthernetPHYWrite(reg, val);
  if (status != kStatus_Success) {
    jsonrpc_return_error(request, -1, "EthernetPHYWrite failed", nullptr);
    return;
  }

  jsonrpc_return_success(request, "{}");
}
}  // namespace

extern "C" void app_main(void* param) {
  coral::micro::InitializeEthernet(false);

  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("eth_get_ip", EthGetIP);
  jsonrpc_export("eth_write_phy", EthWritePHY);
  IperfInit();
  coral::micro::UseHttpServer(new coral::micro::JsonRpcHttpServer);
  vTaskSuspend(nullptr);
}
