#include "libs/RPCServer/rpc_server_io_http.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "libs/base/httpd.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

#define FS_FILE_FLAGS_JSON_RPC (1 << 7)

namespace valiant {
namespace {
int JsonRpcReply(const char* buf, int len, void* userdata) {
    auto* data = reinterpret_cast<JsonRpcHttpServer::Data*>(userdata);
    data->reply_buf.insert(data->reply_buf.end(), buf, buf + len);
    return len;
}

void* FindPointerParam(const char* param, int iNumParams, char** pcParam,
                       char** pcValue) {
    for (int i = 0; i < iNumParams; ++i) {
        if (std::strcmp(param, pcParam[i]) == 0)
            return reinterpret_cast<void*>(
                std::strtoul(pcValue[i], nullptr, 0));
    }
    return nullptr;
}
}  // namespace

err_t JsonRpcHttpServer::PostBegin(void* connection, const char* uri,
                                   const char* http_request,
                                   u16_t http_request_len, int content_len,
                                   char* response_uri, u16_t response_uri_len,
                                   u8_t* post_auto_wnd) {
    if (std::strcmp("/jsonrpc", uri) != 0) return ERR_ARG;

    auto* data = new Data;
    data->post_data.resize(content_len);
    data->post_data_written = 0;
    rpc_data_map_[connection] = data;
    return ERR_OK;
};

err_t JsonRpcHttpServer::PostReceiveData(void* connection, struct pbuf* p) {
    auto* data = rpc_data_map_[connection];
    auto off = data->post_data_written;
    auto len = pbuf_copy_partial(p, data->post_data.data() + off,
                                 data->post_data.size() - off, 0);
    data->post_data_written += len;
    pbuf_free(p);
    return ERR_OK;
}

void JsonRpcHttpServer::PostFinished(void* connection, char* response_uri,
                                     u16_t response_uri_len) {
    auto* data = rpc_data_map_[connection];
    jsonrpc_ctx_process(ctx_, data->post_data.data(), data->post_data.size(),
                        JsonRpcReply, data, nullptr);
    snprintf(response_uri, response_uri_len,
             "/jsonrpc/response.json?connection=%p", connection);
}

void JsonRpcHttpServer::CgiHandler(struct fs_file* file, const char* uri,
                                   int iNumParams, char** pcParam,
                                   char** pcValue) {
    if (file->flags & FS_FILE_FLAGS_JSON_RPC) {
        void* connection =
            FindPointerParam("connection", iNumParams, pcParam, pcValue);
        assert(connection);

        auto* data = rpc_data_map_[connection];

        file->pextension = connection;
        file->data = data->reply_buf.data();
        file->len = data->reply_buf.size();
        file->index = file->len;
        file->flags |= FS_FILE_FLAGS_HEADER_PERSISTENT;
        return;
    }

    HttpServer::CgiHandler(file, uri, iNumParams, pcParam, pcValue);
}

int JsonRpcHttpServer::FsOpenCustom(struct fs_file* file, const char* name) {
    if (std::strcmp("/jsonrpc/response.json", name) == 0) {
        std::memset(file, 0, sizeof(*file));
        file->flags |= FS_FILE_FLAGS_JSON_RPC;
        return 1;
    }

    return HttpServer::FsOpenCustom(file, name);
}

void JsonRpcHttpServer::FsCloseCustom(struct fs_file* file) {
    if (file->flags & FS_FILE_FLAGS_JSON_RPC) {
        auto* data = rpc_data_map_[file->pextension];
        rpc_data_map_.erase(file->pextension);
        delete data;
        return;
    }

    HttpServer::FsCloseCustom(file);
};

}  // namespace valiant
