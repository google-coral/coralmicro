#ifndef _LIBS_BASE_HTTPD_H_
#define _LIBS_BASE_HTTPD_H_

#include <functional>
#include <utility>
#include <vector>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

namespace valiant {
namespace httpd {

class HttpServer {
   public:
    virtual ~HttpServer() = default;

    virtual err_t PostBegin(void* connection, const char* uri,
                            const char* http_request, u16_t http_request_len,
                            int content_len, char* response_uri,
                            u16_t response_uri_len, u8_t* post_auto_wnd) {
        return ERR_ARG;
    }
    virtual err_t PostReceiveData(void* connection, struct pbuf* p) {
        return ERR_ARG;
    };
    virtual void PostFinished(void* connection, char* response_uri,
                              u16_t response_uri_len){};

    virtual void CgiHandler(struct fs_file* file, const char* uri,
                            int iNumParams, char** pcParam, char** pcValue){};

   public:
    virtual int FsOpenCustom(struct fs_file* file, const char* name);
    virtual int FsReadCustom(struct fs_file* file, char* buffer, int count);
    virtual void FsCloseCustom(struct fs_file* file);

   public:
    using DynamicFileHandler =
        std::function<bool(const char* name, std::vector<uint8_t>* buffer)>;

    using StaticFileHandler = std::function<bool(
        const char* name, const uint8_t** buffer, size_t* size)>;

    void SetDynamicFileHandler(DynamicFileHandler handler) {
        dynamic_file_handler_ = std::move(handler);
    }

    void SetStaticFileHandler(StaticFileHandler handler) {
        static_file_handler_ = std::move(handler);
    }

   private:
    DynamicFileHandler dynamic_file_handler_;
    StaticFileHandler static_file_handler_;
};

void Init(HttpServer* server);

}  // namespace httpd
}  // namespace valiant

#endif  // _LIBS_BASE_HTTPD_H_
