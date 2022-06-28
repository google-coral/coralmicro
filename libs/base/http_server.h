/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_BASE_HTTP_SERVER_H_
#define LIBS_BASE_HTTP_SERVER_H_

#include <cstring>
#include <functional>
#include <utility>
#include <variant>
#include <vector>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

namespace coral::micro {

class HttpServer {
   public:
    virtual ~HttpServer() = default;

    virtual err_t PostBegin(void* connection, const char* uri,
                            const char* http_request, u16_t http_request_len,
                            int content_len, char* response_uri,
                            u16_t response_uri_len, u8_t* post_auto_wnd) {
        (void)connection;
        (void)uri;
        (void)http_request;
        (void)http_request_len;
        (void)content_len;
        (void)response_uri;
        (void)response_uri_len;
        (void)post_auto_wnd;
        return ERR_ARG;
    }
    virtual err_t PostReceiveData(void* connection, struct pbuf* p) {
        (void)connection;
        (void)p;
        return ERR_ARG;
    };
    virtual void PostFinished(void* connection, char* response_uri,
                              u16_t response_uri_len) {
        (void)connection;
        (void)response_uri;
        (void)response_uri_len;
    };

    virtual void CgiHandler(struct fs_file* file, const char* uri,
                            int iNumParams, char** pcParam, char** pcValue) {
        (void)file;
        (void)uri;
        (void)iNumParams;
        (void)pcParam;
        (void)pcValue;
    };

    virtual int FsOpenCustom(struct fs_file* file, const char* name);
    virtual int FsReadCustom(struct fs_file* file, char* buffer, int count);
    virtual void FsCloseCustom(struct fs_file* file);

   public:
    struct StaticBuffer {
        const uint8_t* buffer;
        size_t size;
    };

    using Content = std::variant<std::monostate,        // Not found
                                 std::string,           // Filename
                                 std::vector<uint8_t>,  // Dynamic buffer
                                 StaticBuffer>;         // Static buffer

    using UriHandler = std::function<Content(const char* uri)>;

    void AddUriHandler(UriHandler handler) {
        uri_handlers_.push_back(std::move(handler));
    }

   private:
    std::vector<UriHandler> uri_handlers_;
};

void UseHttpServer(HttpServer* server);

}  // namespace coral::micro

#endif  // LIBS_BASE_HTTP_SERVER_H_
