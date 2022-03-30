#include "libs/base/mutex.h"
#include "libs/testlib/test_lib.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/lwiperf.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/tcpip.h"
#include "third_party/mjson/src/mjson.h"

#include <vector>

using valiant::testlib::JsonRpcGetBooleanParam;
using valiant::testlib::JsonRpcGetStringParam;

struct IperfContext {
    SemaphoreHandle_t mutex;
    void *session;
};

static IperfContext iperf_ctx;

static void lwiperf_report(void *arg, enum lwiperf_report_type report_type, const ip_addr_t *local_addr,
                           u16_t local_port, const ip_addr_t *remote_addr, u16_t remote_port,
                           u64_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitspec) {
    valiant::MutexLock lock(iperf_ctx.mutex);
    switch (report_type) {
        case LWIPERF_TCP_DONE_CLIENT:
        case LWIPERF_TCP_ABORTED_REMOTE:
        case LWIPERF_TCP_ABORTED_LOCAL:
            lwiperf_abort(iperf_ctx.session);
            iperf_ctx.session = nullptr;
        default:
            break;
    }
}

static void IperfStart(struct jsonrpc_request *request) {
    valiant::MutexLock lock(iperf_ctx.mutex);
    if (iperf_ctx.session) {
        jsonrpc_return_error(request, -1, "iperf is already running!", nullptr);
        return;
    }

    bool server;
    if (!JsonRpcGetBooleanParam(request, "is_server", &server)) return;

    if (server) {
        iperf_ctx.session = lwiperf_start_tcp_server_default(lwiperf_report, &iperf_ctx);
    } else { // client
        std::string server_ip_address;
        if (!JsonRpcGetStringParam(request, "server_ip_address", &server_ip_address)) return;
        ip_addr_t addr;
        if (ipaddr_aton(server_ip_address.c_str(), &addr) == -1) {
            jsonrpc_return_error(request, -1, "failed to parse `server_ip_address`", nullptr);
            return;
        }
        iperf_ctx.session = lwiperf_start_tcp_client_default(&addr, lwiperf_report, &iperf_ctx);
    }

    if (iperf_ctx.session) {
        jsonrpc_return_success(request, "{}");
    } else {
        jsonrpc_return_error(request, -1, "iperf failed to start", nullptr);
    }
}

static void IperfStop(struct jsonrpc_request *request) {
    valiant::MutexLock lock(iperf_ctx.mutex);
    if (!iperf_ctx.session) {
        jsonrpc_return_error(request, -1, "iperf is already stopped!", nullptr);
        return;
    }
    lwiperf_abort(iperf_ctx.session);
    iperf_ctx.session = nullptr;
    jsonrpc_return_success(request, "{}");
}

void IperfInit() {
    iperf_ctx.mutex = xSemaphoreCreateMutex();
    iperf_ctx.session = nullptr;
    jsonrpc_export("iperf_start", IperfStart);
    jsonrpc_export("iperf_stop", IperfStop);
}

