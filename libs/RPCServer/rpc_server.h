#ifndef _LIBS_RPCSERVER_RPC_SERVER_H_
#define _LIBS_RPCSERVER_RPC_SERVER_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/mjson/src/mjson.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace valiant {
namespace rpc {

class RPCServerIO {
  public:
    RPCServerIO() = default;
    ~RPCServerIO() = default;
    virtual bool Init() = 0;
    struct jsonrpc_ctx* parent_context() { return ctx_; }

    void SetContext(struct jsonrpc_ctx* ctx) {
      ctx_ = ctx;
    }
  private:
    struct jsonrpc_ctx* ctx_;
};

}  // namespace rpc
}  // namespace valiant

#endif  // _LIBS_RPCSERVER_RPC_SERVER_H_
