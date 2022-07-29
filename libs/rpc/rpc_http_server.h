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

#ifndef LIBS_RPC_RPC_HTTP_SERVER_H_
#define LIBS_RPC_RPC_HTTP_SERVER_H_

#include <map>
#include <vector>

#include "libs/base/http_server.h"
#include "third_party/mjson/src/mjson.h"

namespace coralmicro {

class JsonRpcHttpServer : public coralmicro::HttpServer {
 public:
  explicit JsonRpcHttpServer(struct jsonrpc_ctx* ctx = &jsonrpc_default_context)
      : ctx_(ctx) {}

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
  std::map<void*, std::vector<char>> buffers_;  // connection-to-buffer map
};

}  // namespace coralmicro

#endif  // LIBS_RPC_RPC_HTTP_SERVER_H_
