#ifndef _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_
#define _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_

#include "libs/base/httpd.h"
#include "libs/RPCServer/rpc_server.h"

#include <map>
#include <vector>

namespace valiant {
namespace rpc {

class RPCServerIOHTTP : public RPCServerIO {
  public:
    RPCServerIOHTTP() = default;
    ~RPCServerIOHTTP() = default;
    bool Init() override;

    struct rpc_data {
        std::vector<char> post_data;
        std::vector<char> reply_buf;
        size_t post_data_written;
    };
  private:
    err_t httpd_post_begin(void* connection, const char* uri, const char* http_request,
                           u16_t http_request_len, int content_len, char* response_uri,
                           u16_t response_uri_len, u8_t* post_auto_wnd);
    void httpd_post_finished(void* connection, char* response_uri, u16_t response_uri_len);
    err_t httpd_post_receive_data(void* connection, struct pbuf* p);
    void httpd_cgi_handler(struct fs_file* file, const char* uri, int iNumParams, char** pcParam, char** pcValue);
    int fs_read_custom(struct fs_file* file, char* buffer, int count);
    int fs_open_custom(struct fs_file* file, const char* name);
    void fs_close_custom(struct fs_file* file);

    static err_t static_httpd_post_begin(void* context,
                                         void* connection, const char* uri, const char* http_request,
                                         u16_t http_request_len, int content_len, char* response_uri,
                                         u16_t response_uri_len, u8_t* post_auto_wnd);
    static void static_httpd_post_finished(void* context, void* connection, char* response_uri, u16_t response_uri_len);
    static err_t static_httpd_post_receive_data(void* context, void* connection, struct pbuf* p);
    static void static_httpd_cgi_handler(void* context, struct fs_file* file, const char* uri, int iNumParams, char** pcParam, char** pcValue);
    static int static_fs_read_custom(void* context, struct fs_file* file, char* buffer, int count);
    static int static_fs_open_custom(void* context, struct fs_file* file, const char* name);
    static void static_fs_close_custom(void* context, struct fs_file *file);
    static int jsonrpc_reply(const char *buf, int len, void *userdata);

    valiant::httpd::HTTPDHandlers handlers_;
    std::map<void*, rpc_data*> rpc_data_map;
};

}  // namespace rpc
}  // namespace valiant

#endif  // _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_
