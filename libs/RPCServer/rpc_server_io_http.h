#ifndef _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_
#define _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_

#include <map>
#include <vector>

#include "libs/base/httpd.h"
#include "third_party/mjson/src/mjson.h"

namespace valiant {
namespace rpc {

class RpcServer {
   public:
    RpcServer() = default;
    ~RpcServer() = default;

    bool Init(struct jsonrpc_ctx* ctx);

    struct Data {
        std::vector<char> post_data;
        std::vector<char> reply_buf;
        size_t post_data_written;
    };

   private:
    err_t httpd_post_begin(void* connection, const char* uri,
                           const char* http_request, u16_t http_request_len,
                           int content_len, char* response_uri,
                           u16_t response_uri_len, u8_t* post_auto_wnd);
    void httpd_post_finished(void* connection, char* response_uri,
                             u16_t response_uri_len);
    err_t httpd_post_receive_data(void* connection, struct pbuf* p);
    void httpd_cgi_handler(struct fs_file* file, const char* uri,
                           int iNumParams, char** pcParam, char** pcValue);
    int fs_open_custom(struct fs_file* file, const char* name);
    void fs_close_custom(struct fs_file* file);

    struct jsonrpc_ctx* ctx_;
    valiant::httpd::HTTPDHandlers handlers_;
    std::map<void*, Data*> rpc_data_map_;
};

}  // namespace rpc
}  // namespace valiant

#endif  // _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_
