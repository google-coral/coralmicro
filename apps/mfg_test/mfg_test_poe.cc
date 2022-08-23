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

#include "apps/mfg_test/mfg_test_iperf.h"
#include "libs/base/ethernet.h"
#include "libs/base/main_freertos_m7.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"

namespace {
void EthGetIP(struct jsonrpc_request* request) {
  struct netif* ethernet = coralmicro::EthernetGetInterface();
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

void EthWritePhy(struct jsonrpc_request* request) {
  int reg;
  if (!coralmicro::JsonRpcGetIntegerParam(request, "reg", &reg)) return;

  int val;
  if (!coralmicro::JsonRpcGetIntegerParam(request, "val", &val)) return;

  status_t status = coralmicro::EthernetPhyWrite(reg, val);
  if (status != kStatus_Success) {
    jsonrpc_return_error(request, -1, "EthernetPHYWrite failed", nullptr);
    return;
  }

  jsonrpc_return_success(request, "{}");
}
}  // namespace

extern "C" void app_main(void* param) {
  coralmicro::EthernetInit(/*default_iface=*/false);

  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("eth_get_ip", EthGetIP);
  jsonrpc_export("eth_write_phy", EthWritePhy);
  IperfInit();
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
  vTaskSuspend(nullptr);
}
