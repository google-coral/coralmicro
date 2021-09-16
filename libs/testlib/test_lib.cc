#include "libs/base/utils.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testconv1/testconv1.h"
#include "libs/testlib/test_lib.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <array>

namespace valiant {
namespace testlib {

static std::unique_ptr<char> JSONRPCCreateParamFormatString(const char *param_name) {
    const char *param_format = "$[0].%s";
    // +1 for null terminator.
    int param_pattern_len = snprintf(nullptr, 0, param_format, param_name) + 1;
    std::unique_ptr<char> param_pattern(new char[param_pattern_len]);
    snprintf(param_pattern.get(), param_pattern_len, param_format, param_name);
    return param_pattern;
}

bool JSONRPCGetIntegerParam(struct jsonrpc_request* request, const char *param_name, int* out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);
    double param_double;
    int find_result = mjson_find(request->params, request->params_len, param_pattern.get(), nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, param_pattern.get(), &param_double);
        *out = static_cast<int>(param_double);
    } else {
        return false;
    }
    return true;
}

bool JSONRPCGetBooleanParam(struct jsonrpc_request* request, const char *param_name, bool *out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);
    int find_result = mjson_find(request->params, request->params_len, param_pattern.get(), nullptr, nullptr);
    if (find_result == MJSON_TOK_TRUE || find_result == MJSON_TOK_FALSE) {
        int param;
        mjson_get_bool(request->params, request->params_len, param_pattern.get(), &param);
        *out = !!param;
    } else {
        return false;
    }
    return true;
}

bool JSONRPCGetStringParam(struct jsonrpc_request* request, const char *param_name, std::vector<char>* out) {
    int find_result;
    ssize_t find_size = 0;
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);

    find_result = mjson_find(request->params, request->params_len, param_pattern.get(), nullptr, &find_size);
    if (find_result == MJSON_TOK_STRING) {
        out->resize(find_size, 0);
        mjson_get_string(request->params, request->params_len, param_pattern.get(), out->data(), find_size);
    } else {
        return false;
    }
    return true;
}

// Implementation of "get_serial_number" RPC.
// Returns JSON results with the key "serial_number" and the serial, as a string.
void GetSerialNumber(struct jsonrpc_request *request) {
    char serial_number_str[16];
    uint64_t serial_number = valiant::utils::GetUniqueID();
    for (int i = 0; i < 16; ++i) {
        uint8_t nibble = (serial_number >> (i * 4)) & 0xF;
        if (nibble < 10) {
            serial_number_str[15 - i] = nibble + '0';
        } else {
            serial_number_str[15 - i] = (nibble - 10) + 'a';
        }
    }
    jsonrpc_return_success(request, "{%Q:%.*Q}", "serial_number", 16, serial_number_str);
}

// Implements the "run_testconv1" RPC.
// Runs the simple "testconv1" model using the TPU.
// NOTE: The TPU power must be enabled for this RPC to succeed.
void RunTestConv1(struct jsonrpc_request *request) {
    if (!valiant::EdgeTpuTask::GetSingleton()->GetPower()) {
        jsonrpc_return_error(request, -1, "TPU power is not enabled", nullptr);
        return;
    }
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!valiant::testconv1::setup()) {
        jsonrpc_return_error(request, -1, "testconv1 setup failed", nullptr);
        return;
    }
    if (!valiant::testconv1::loop()) {
        jsonrpc_return_error(request, -1, "testconv1 loop failed", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{}");
}

// Implements the "set_tpu_power_state" RPC.
// Takes one parameter, "enable" -- a boolean indicating the state to set.
// Returns success or failure.
void SetTPUPowerState(struct jsonrpc_request *request) {
    bool enable;
    if (!JSONRPCGetBooleanParam(request, "enable", &enable)) {
        jsonrpc_return_error(request, -1, "'enable' missing or invalid", nullptr);
        return;
    }

    valiant::EdgeTpuTask::GetSingleton()->SetPower(enable);
    jsonrpc_return_success(request, "{}");
}

}  // namespace testlib
}  // namespace valiant
