#include "libs/base/mutex.h"
#include "libs/RPCServer/rpc_server.h"
#include "third_party/mjson/src/mjson.h"

namespace valiant {
namespace rpc {

RPCServer::~RPCServer() {
    vSemaphoreDelete(mu_);
}

bool RPCServer::Init() {
    mu_ = xSemaphoreCreateMutex();
    if (!mu_) {
        return false;
    }

    jsonrpc_ctx_init(&ctx_, nullptr, this);
    jsonrpc_ctx_export(&ctx_, MJSON_RPC_LIST_NAME, jsonrpc_list);
    return true;
}

void RPCServer::RegisterRPC(const std::string& name, RPCHandler handler) {
    MutexLock lock(mu_);

    struct jsonrpc_method *m = new jsonrpc_method;
    std::string *n = new std::string(name);

    rpc_methods.push_back(std::unique_ptr<struct jsonrpc_method>(m));
    rpc_names.push_back(std::unique_ptr<std::string>(n));

    m->method = n->c_str();
    m->method_sz = n->length();
    m->cb = handler;
    m->next = ctx_.methods;
    ctx_.methods = m;
}

void RPCServer::RegisterIO(RPCServerIO& io) {
    io.SetParent(this);
}

struct jsonrpc_ctx* RPCServerIO::parent_context() {
    if (parent_) {
        return parent_->context();
    }
    return nullptr;
}

}  // namespace rpc
}  // namespace valiant
