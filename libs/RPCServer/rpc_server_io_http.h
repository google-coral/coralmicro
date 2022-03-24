#ifndef _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_
#define _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_

#include <map>
#include <vector>

#include "libs/base/httpd.h"
#include "third_party/mjson/src/mjson.h"

namespace valiant {

class JsonRpcHttpServer : public valiant::httpd::HttpServer {
   public:
    JsonRpcHttpServer(struct jsonrpc_ctx* ctx = &jsonrpc_default_context)
        : ctx_(ctx) {}

    struct Data {
        std::vector<char> post_data;
        std::vector<char> reply_buf;
        size_t post_data_written;
    };

    err_t PostBegin(void* connection, const char* uri, const char* http_request,
                    u16_t http_request_len, int content_len, char* response_uri,
                    u16_t response_uri_len, u8_t* post_auto_wnd) override;
    err_t PostReceiveData(void* connection, struct pbuf* p) override;
    void PostFinished(void* connection, char* response_uri,
                      u16_t response_uri_len) override;

    void CgiHandler(struct fs_file* file, const char* uri, int iNumParams,
                    char** pcParam, char** pcValue) override;

    int FsOpenCustom(struct fs_file* file, const char* name) override;
    void FsCloseCustom(struct fs_file* file) override;

   private:
    struct jsonrpc_ctx* ctx_;
    std::map<void*, Data*> rpc_data_map_;
};

}  // namespace valiant

#endif  // _LIBS_RPC_SERVER_RPC_SERVER_IO_HTTP_H_
