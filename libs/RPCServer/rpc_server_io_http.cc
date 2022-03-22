#include "libs/RPCServer/rpc_server_io_http.h"

#include <cstring>

#include "libs/base/httpd.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

namespace valiant {
namespace rpc {
namespace {
RpcServer* self(void* context) {
    return reinterpret_cast<valiant::rpc::RpcServer*>(context);
}

int jsonrpc_reply(const char* buf, int len, void* userdata) {
    auto* data = reinterpret_cast<RpcServer::Data*>(userdata);
    data->reply_buf.insert(data->reply_buf.end(), buf, buf + len);
    return len;
}
}  // namespace

err_t RpcServer::httpd_post_receive_data(void* connection, struct pbuf* p) {
    auto* data = rpc_data_map_[connection];
    auto off = data->post_data_written;
    auto len = pbuf_copy_partial(p, data->post_data.data() + off,
                                 data->post_data.size() - off, 0);
    data->post_data_written += len;
    pbuf_free(p);
    return ERR_OK;
}

err_t RpcServer::httpd_post_begin(void* connection, const char* uri,
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
}

void RpcServer::httpd_post_finished(void* connection, char* response_uri,
                                    u16_t response_uri_len) {
    auto* data = rpc_data_map_[connection];
    jsonrpc_ctx_process(ctx_, data->post_data.data(), data->post_data.size(),
                        jsonrpc_reply, data, nullptr);
    const char* response = "/jsonrpc/response.json?connection=%x";
    snprintf(response_uri, response_uri_len, response, connection);
}

void RpcServer::httpd_cgi_handler(struct fs_file* file, const char* uri,
                                  int iNumParams, char** pcParam,
                                  char** pcValue) {
    void* connection = file->pextension;
    auto* data = rpc_data_map_[connection];

    file->len = data->reply_buf.size();
    file->data = data->reply_buf.data();
    file->index = file->len;
    file->flags |= FS_FILE_FLAGS_HEADER_PERSISTENT;
}

int RpcServer::fs_open_custom(struct fs_file* file, const char* name) {
    if (std::strcmp("/jsonrpc/response.json", name) == 0) {
        memset(file, 0, sizeof(*file));
        return 1;
    }
    return 0;
}

void RpcServer::fs_close_custom(struct fs_file* file) {
    void* connection = file->pextension;
    if (!connection) return;

    auto* data = rpc_data_map_[connection];
    if (data) {
        delete data;
        rpc_data_map_.erase(connection);
    }
}

bool RpcServer::Init(struct jsonrpc_ctx* ctx) {
    ctx_ = ctx;

    handlers_.post_begin =
        +[](void* context, void* connection, const char* uri,
            const char* http_request, u16_t http_request_len, int content_len,
            char* response_uri, u16_t response_uri_len, u8_t* post_auto_wnd) {
            return self(context)->httpd_post_begin(
                connection, uri, http_request, http_request_len, content_len,
                response_uri, response_uri_len, post_auto_wnd);
        };
    handlers_.post_finished = +[](void* context, void* connection,
                                  char* response_uri, u16_t response_uri_len) {
        self(context)->httpd_post_finished(connection, response_uri,
                                           response_uri_len);
    };
    handlers_.post_receive_data =
        +[](void* context, void* connection, struct pbuf* p) {
            return self(context)->httpd_post_receive_data(connection, p);
        };
    handlers_.cgi_handler =
        +[](void* context, struct fs_file* file, const char* uri,
            int iNumParams, char** pcParam, char** pcValue) {
            self(context)->httpd_cgi_handler(file, uri, iNumParams, pcParam,
                                             pcValue);
        };
    handlers_.fs_open_custom =
        +[](void* context, struct fs_file* file, const char* name) {
            return self(context)->fs_open_custom(file, name);
        };
    handlers_.fs_read_custom =
        +[](void* context, struct fs_file* file, char* buffer, int count) {
            return FS_READ_EOF;
        };
    handlers_.fs_close_custom = +[](void* context, struct fs_file* file) {
        self(context)->fs_close_custom(file);
    };
    handlers_.context = this;

    valiant::httpd::Init();
    return valiant::httpd::RegisterHandlerForPath("/jsonrpc", &handlers_);
}

}  // namespace rpc
}  // namespace valiant
