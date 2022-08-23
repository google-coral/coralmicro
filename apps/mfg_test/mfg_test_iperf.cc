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

#include <vector>

#include "libs/base/check.h"
#include "libs/base/mutex.h"
#include "libs/rpc/rpc_utils.h"
#include "third_party/freertos_kernel/include/timers.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/lwiperf.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/inet.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/tcpip.h"

/*  -u (UDP), -i (interval), -l (data length) and -b (bandwidth) */

namespace {
using coralmicro::JsonRpcGetBooleanParam;
using coralmicro::JsonRpcGetIntegerParam;
using coralmicro::JsonRpcGetStringParam;

struct IperfContext {
  SemaphoreHandle_t mutex;
  TimerHandle_t poll_timer;
  void* session;
};

IperfContext iperf_ctx;

void lwiperf_report(void* arg, enum lwiperf_report_type report_type,
                    const ip_addr_t* local_addr, u16_t local_port,
                    const ip_addr_t* remote_addr, u16_t remote_port,
                    u64_t bytes_transferred, u32_t ms_duration,
                    u32_t bandwidth_kbitspec) {
  coralmicro::MutexLock lock(iperf_ctx.mutex);
  switch (report_type) {
    case LWIPERF_TCP_ABORTED_REMOTE:
    case LWIPERF_UDP_ABORTED_REMOTE:
      lwiperf_abort(iperf_ctx.session);
    case LWIPERF_TCP_DONE_CLIENT:
    case LWIPERF_UDP_DONE_CLIENT:
      iperf_ctx.session = nullptr;
    default:
      break;
  }
}

void poll_udp_client(void* arg) { lwiperf_poll_udp_client(); }

void timer_poll_udp_client(TimerHandle_t timer) {
  tcpip_try_callback(poll_udp_client, nullptr);
}

void IperfStart(struct jsonrpc_request* request) {
  coralmicro::MutexLock lock(iperf_ctx.mutex);
  if (iperf_ctx.session) {
    jsonrpc_return_error(request, -1, "iperf is already running!", nullptr);
    return;
  }

  bool server;
  if (!JsonRpcGetBooleanParam(request, "is_server", &server)) return;

  bool udp = false;
  JsonRpcGetBooleanParam(request, "udp", &udp);

  if (server) {
    if (udp) {
      iperf_ctx.session = lwiperf_start_udp_server(
          &ip_addr_any, LWIPERF_TCP_PORT_DEFAULT, lwiperf_report, &iperf_ctx);
    } else {
      iperf_ctx.session =
          lwiperf_start_tcp_server_default(lwiperf_report, &iperf_ctx);
    }
  } else {  // client
    std::string server_ip_address;
    if (!JsonRpcGetStringParam(request, "server_ip_address",
                               &server_ip_address))
      return;

    int duration_seconds;
    if (!JsonRpcGetIntegerParam(request, "duration", &duration_seconds)) {
      duration_seconds = 10;
    }
    // Per lwiperf code, positive value for tcp_client's `amount` is bytes,
    // negative value are time (unit of 10ms)
    int amount = -(duration_seconds * 100);

    ip_addr_t addr;
    if (ipaddr_aton(server_ip_address.c_str(), &addr) == -1) {
      jsonrpc_return_error(request, -1, "failed to parse `server_ip_address`",
                           nullptr);
      return;
    }
    if (udp) {
      int bandwidth = 1024 * 1024;
      JsonRpcGetIntegerParam(request, "bandwidth", &bandwidth);
      iperf_ctx.session = lwiperf_start_udp_client(
          &ip_addr_any, LWIPERF_TCP_PORT_DEFAULT, &addr,
          LWIPERF_TCP_PORT_DEFAULT, LWIPERF_CLIENT, amount, bandwidth, 0,
          lwiperf_report, &iperf_ctx);
    } else {
      iperf_ctx.session = lwiperf_start_tcp_client(
          &addr, LWIPERF_TCP_PORT_DEFAULT, LWIPERF_CLIENT, amount,
          lwiperf_report, &iperf_ctx);
    }
  }

  if (iperf_ctx.session) {
    jsonrpc_return_success(request, "{}");
  } else {
    jsonrpc_return_error(request, -1, "iperf failed to start", nullptr);
  }
}

void IperfStop(struct jsonrpc_request* request) {
  coralmicro::MutexLock lock(iperf_ctx.mutex);
  if (!iperf_ctx.session) {
    jsonrpc_return_error(request, -1, "iperf is already stopped!", nullptr);
    return;
  }
  lwiperf_abort(iperf_ctx.session);
  iperf_ctx.session = nullptr;
  jsonrpc_return_success(request, "{}");
}
}  // namespace

void IperfInit() {
  iperf_ctx.mutex = xSemaphoreCreateMutex();
  CHECK(iperf_ctx.mutex);
  iperf_ctx.session = nullptr;

  iperf_ctx.poll_timer = xTimerCreate("UDP poll timer", 1 / portTICK_PERIOD_MS,
                                      pdTRUE, nullptr, timer_poll_udp_client);
  CHECK(iperf_ctx.poll_timer);
  CHECK(xTimerStart(iperf_ctx.poll_timer, 0) == pdPASS);

  jsonrpc_export("iperf_start", IperfStart);
  jsonrpc_export("iperf_stop", IperfStop);
}
