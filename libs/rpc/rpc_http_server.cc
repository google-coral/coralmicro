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

#include "libs/rpc/rpc_http_server.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

#define FS_FILE_FLAGS_JSON_RPC (1 << 7)

namespace coralmicro {
namespace {
int Append(const char* buf, int len, void* userdata) {
  auto* v = static_cast<std::vector<char>*>(userdata);
  v->insert(v->end(), buf, buf + len);
  return len;
}

void* FindPointerParam(const char* param, int iNumParams, char** pcParam,
                       char** pcValue) {
  for (int i = 0; i < iNumParams; ++i) {
    if (std::strcmp(param, pcParam[i]) == 0)
      return reinterpret_cast<void*>(std::strtoul(pcValue[i], nullptr, 0));
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

  buffers_[connection].reserve(content_len);
  return ERR_OK;
};

err_t JsonRpcHttpServer::PostReceiveData(void* connection, struct pbuf* p) {
  auto& buf = buffers_[connection];
  auto off = buf.size();
  buf.resize(buf.size() + p->tot_len);
  auto len = pbuf_copy_partial(p, buf.data() + off, buf.size() - off, 0);
  assert(len == p->tot_len);
  pbuf_free(p);
  return ERR_OK;
}

void JsonRpcHttpServer::PostFinished(void* connection, char* response_uri,
                                     u16_t response_uri_len) {
  auto& buf = buffers_[connection];
  std::vector<char> reply;
  jsonrpc_ctx_process(ctx_, buf.data(), buf.size(), Append, &reply, nullptr);
  buf = std::move(reply);
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

    auto& buf = buffers_[connection];

    file->pextension = connection;
    file->data = buf.data();
    file->len = buf.size();
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
    buffers_.erase(file->pextension);
    return;
  }

  HttpServer::FsCloseCustom(file);
};

}  // namespace coralmicro
