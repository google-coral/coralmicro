#include "libs/base/httpd.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

namespace valiant {
namespace rpc {

int RPCServerIOHTTP::jsonrpc_reply(const char *buf, int len, void *userdata) {
    rpc_data *this_rpc_data = reinterpret_cast<rpc_data*>(userdata);
    for (int i = 0; i < len; ++i) {
        this_rpc_data->reply_buf.push_back(buf[i]);
    }
    return len;
}

err_t RPCServerIOHTTP::static_httpd_post_receive_data(void *context, void *connection, struct pbuf *p) {
    valiant::rpc::RPCServerIOHTTP* rpc_server_io_http =
        reinterpret_cast<valiant::rpc::RPCServerIOHTTP*>(context);
    return rpc_server_io_http->httpd_post_receive_data(connection, p);
}

err_t RPCServerIOHTTP::httpd_post_receive_data(void *connection, struct pbuf *p) {
    auto *this_rpc_data = rpc_data_map[connection];
    auto post_data_len = this_rpc_data->post_data_written;
    if (p->len == p->tot_len) {
        memcpy(this_rpc_data->post_data.data() + post_data_len, p->payload, p->tot_len);
    } else {
        size_t offset = post_data_len;
        struct pbuf *tmp_p = p;
        do {
            memcpy(this_rpc_data->post_data.data() + offset, tmp_p->payload, tmp_p->len);
            offset += tmp_p->len;
            tmp_p = tmp_p->next;
        } while (tmp_p->len != tmp_p->tot_len);
    }
    this_rpc_data->post_data_written += p->tot_len;
    pbuf_free(p);
    return ERR_OK;
}

err_t RPCServerIOHTTP::static_httpd_post_begin(void* context,
                       void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd) {
    valiant::rpc::RPCServerIOHTTP* rpc_server_io_http =
        reinterpret_cast<valiant::rpc::RPCServerIOHTTP*>(context);
    return rpc_server_io_http->httpd_post_begin(connection, uri, http_request,
                                                http_request_len, content_len, response_uri,
                                                response_uri_len, post_auto_wnd);
}

err_t RPCServerIOHTTP::httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd) {
    auto *this_rpc_data = new rpc_data;
    this_rpc_data->post_data.resize(content_len, 0);
    this_rpc_data->reply_buf_written = 0;
    this_rpc_data->post_data_written = 0;
    rpc_data_map[connection] = this_rpc_data;
    const char *expected_uri = "/jsonrpc";
    if (strncmp(uri, expected_uri, strlen(expected_uri)) == 0) {
        return ERR_OK;
    }
    return ERR_ARG;
}

void RPCServerIOHTTP::static_httpd_post_finished(void* context, void *connection, char *response_uri, u16_t response_uri_len) {
    valiant::rpc::RPCServerIOHTTP* rpc_server_io_http =
        reinterpret_cast<valiant::rpc::RPCServerIOHTTP*>(context);
    rpc_server_io_http->httpd_post_finished(connection, response_uri, response_uri_len);
}

void RPCServerIOHTTP::httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    auto *this_rpc_data = rpc_data_map[connection];
    jsonrpc_ctx_process(parent_context(), this_rpc_data->post_data.data(), this_rpc_data->post_data.size(), jsonrpc_reply, this_rpc_data, nullptr);
    const char *response = "/jsonrpc/response.json?connection=%x";
    snprintf(response_uri, response_uri_len, response, connection);
}

void RPCServerIOHTTP::static_httpd_cgi_handler(void* context, struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue) {
    valiant::rpc::RPCServerIOHTTP* rpc_server_io_http =
        reinterpret_cast<valiant::rpc::RPCServerIOHTTP*>(context);
    rpc_server_io_http->httpd_cgi_handler(file, uri, iNumParams, pcParam, pcValue);
}

void RPCServerIOHTTP::httpd_cgi_handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue) {
    void *connection = file->pextension;
    auto *this_rpc_data = rpc_data_map[connection];
    file->len = this_rpc_data->reply_buf.size();
    file->index = 0;
}

int RPCServerIOHTTP::static_fs_read_custom(void* context, struct fs_file *file, char *buffer, int count) {
    valiant::rpc::RPCServerIOHTTP* rpc_server_io_http =
        reinterpret_cast<valiant::rpc::RPCServerIOHTTP*>(context);
    return rpc_server_io_http->fs_read_custom(file, buffer, count);
}

int RPCServerIOHTTP::fs_read_custom(struct fs_file *file, char *buffer, int count) {
    void *connection = file->pextension;
    if (!connection) {
        return ERR_ARG;
    }
    auto *this_rpc_data = rpc_data_map[connection];
    if (!this_rpc_data) {
        return FS_READ_EOF;
    }

    size_t bytes_to_read = std::min(static_cast<size_t>(count), this_rpc_data->reply_buf.size() - this_rpc_data->reply_buf_written);

    // We've read all the bytes in our file, let the webserver know this is all the bytes.
    if (this_rpc_data->reply_buf_written == this_rpc_data->reply_buf.size()) {
        return FS_READ_EOF;
    }

    memcpy(buffer, &this_rpc_data->reply_buf[this_rpc_data->reply_buf_written], bytes_to_read);
    this_rpc_data->reply_buf_written += bytes_to_read;
    return bytes_to_read;
}

int RPCServerIOHTTP::static_fs_open_custom(void* context, struct fs_file *file, const char *name) {
    valiant::rpc::RPCServerIOHTTP* rpc_server_io_http =
        reinterpret_cast<valiant::rpc::RPCServerIOHTTP*>(context);
    return rpc_server_io_http->fs_open_custom(file, name);
}

int RPCServerIOHTTP::fs_open_custom(struct fs_file *file, const char *name) {
    const char *response_uri = "/jsonrpc/response.json";
    if (strncmp(name, response_uri, strlen(response_uri)) == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->index = -1;
        return 1;
    }
    return 0;
}

void RPCServerIOHTTP::static_fs_close_custom(void* context, struct fs_file *file) {
    valiant::rpc::RPCServerIOHTTP* rpc_server_io_http =
        reinterpret_cast<valiant::rpc::RPCServerIOHTTP*>(context);
    rpc_server_io_http->fs_close_custom(file);
}

void RPCServerIOHTTP::fs_close_custom(struct fs_file *file) {
    void *connection = file->pextension;
    if (!connection) {
        return;
    }
    auto *this_rpc_data = rpc_data_map[connection];
    if (this_rpc_data) {
        delete this_rpc_data;
        rpc_data_map.erase(connection);
    }
}

bool RPCServerIOHTTP::Init() {
    handlers_.post_begin = static_httpd_post_begin;
    handlers_.post_finished = static_httpd_post_finished;
    handlers_.post_receive_data = static_httpd_post_receive_data;
    handlers_.cgi_handler = static_httpd_cgi_handler;
    handlers_.fs_open_custom = static_fs_open_custom;
    handlers_.fs_read_custom = static_fs_read_custom;
    handlers_.fs_close_custom = static_fs_close_custom;
    handlers_.context = this;

    valiant::httpd::Init();
    return valiant::httpd::RegisterHandlerForPath("/jsonrpc", &handlers_);
}

}  // namespace rpc
}  // namespace valiant
