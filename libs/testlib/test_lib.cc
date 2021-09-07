#include "libs/testlib/test_lib.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/base/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <array>

namespace valiant {
namespace testlib {

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

}  // namespace testlib
}  // namespace valiant
