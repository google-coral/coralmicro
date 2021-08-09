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

class RPCServer;
class RPCServerIO;

class RPCServerIO {
  public:
    RPCServerIO() = default;
    ~RPCServerIO() = default;
    virtual bool Init() = 0;
    struct jsonrpc_ctx* parent_context();
  private:
    void SetParent(RPCServer* s) { parent_ = s; }
    friend class RPCServer;
    RPCServer *parent_;
};

class RPCServer {
  public:
    typedef void (*RPCHandler)(struct jsonrpc_request*);
    RPCServer() = default;
    ~RPCServer();

    bool Init();
    void RegisterRPC(const std::string& name, RPCHandler handler);
    void RegisterIO(RPCServerIO& io);
    struct jsonrpc_ctx* context() { return &ctx_; }
  private:
    SemaphoreHandle_t mu_;
    struct jsonrpc_ctx ctx_;
    std::vector<std::unique_ptr<std::string>> rpc_names;
    std::vector<std::unique_ptr<struct jsonrpc_method>> rpc_methods;
};

}  // namespace rpc
}  // namespace valiant

#endif  // _LIBS_RPCSERVER_RPC_SERVER_H_
