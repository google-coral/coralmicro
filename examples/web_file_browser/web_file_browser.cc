// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/base/http_server.h"
#include "libs/base/led.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Hosts an HTTP server on the Dev Board Micro, allowing connected clients
// to browse files with a web browser, and push/pull files to the board with
// curl commands.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e web_file_browser

namespace coralmicro {
namespace {

constexpr char kUrlPrefix[] = "/fs/";

void RemoveDuplicateChar(std::string& s, char ch) {
  s.erase(std::unique(s.begin(), s.end(),
                      [ch](char a, char b) { return a == ch && b == ch; }),
          s.end());
}

std::string GetPath(const char* uri) {
  static constexpr char kIndexShtml[] = "index.shtml";
  std::string path(uri);
  RemoveDuplicateChar(path, '/');

  if (StrEndsWith(path, kIndexShtml))
    return path.substr(0, path.size() - StrLen(kIndexShtml));
  return path;
}

bool DirHtml(lfs_dir_t* dir, const char* dirname, std::vector<uint8_t>* html) {
  StrAppend(html, "<!DOCTYPE html>\r\n");
  StrAppend(html, "<html lang=\"en\">\r\n");
  StrAppend(html, "<head>\r\n");
  StrAppend(html,
            "<meta http-equiv=\"Content-Type\" content=\"text/html; "
            "charset=utf-8\">\r\n");
  StrAppend(html, "<title>Directory listing for %s</title>\r\n", dirname);
  StrAppend(html, "</head>\r\n");
  StrAppend(html, "<body>\r\n");
  StrAppend(html, "<h1>Directory listing for %s</h1>\r\n", dirname);
  StrAppend(html, "<hr>\r\n");
  StrAppend(html, "<table>\r\n");
  int res;
  lfs_info info;
  while ((res = lfs_dir_read(Lfs(), dir, &info)) != 0) {
    if (res < 0) return false;

    if (info.type == LFS_TYPE_REG) {
      StrAppend(html, "<tr>\r\n");
      StrAppend(html, "<td>&#128462;</td>\r\n");
      StrAppend(html, "<td><a href=\"%s%s%s\">%s</a></td>\r\n", kUrlPrefix,
                dirname + 1, info.name, info.name);
      StrAppend(html, "<td>%d</td>", info.size);
      StrAppend(html, "</tr>\r\n");
    } else if (info.type == LFS_TYPE_DIR) {
      if (std::strcmp(info.name, ".") == 0) continue;
      StrAppend(html, "<tr>\r\n");
      StrAppend(html, "<td>&#128193;</td>\r\n");
      StrAppend(html, "<td><a href=\"%s%s%s/\">%s/</a></td>\r\n", kUrlPrefix,
                dirname + 1, info.name, info.name);
      StrAppend(html, "<td></td>\r\n");
      StrAppend(html, "</tr>\r\n");
    }
  }
  StrAppend(html, "</table>\r\n");
  StrAppend(html, "<hr>\r\n");
  StrAppend(html, "</body>\r\n");
  StrAppend(html, "</html>\r\n");
  return true;
}

HttpServer::Content UriHandler(const char* uri) {
  if (!StrStartsWith(uri, kUrlPrefix)) return {};
  auto path = GetPath(uri + StrLen(kUrlPrefix) - 1);
  assert(!path.empty() && path.front() == '/');

  printf("GET %s => %s\r\n", uri, path.c_str());

  lfs_dir_t dir;
  int res = lfs_dir_open(Lfs(), &dir, path.c_str());
  if (res < 0) {
    if (LfsFileExists(path.c_str())) return path;
    return {};
  }

  // Make sure directory path has '/' at the end.
  if (path.back() != '/') path.push_back('/');

  std::vector<uint8_t> html;
  html.reserve(1024);
  if (!DirHtml(&dir, path.c_str(), &html)) {
    lfs_dir_close(Lfs(), &dir);
    return {};
  }

  lfs_dir_close(Lfs(), &dir);
  return html;
}

class FileHttpServer : public HttpServer {
 public:
  err_t PostBegin(void* connection, const char* uri, const char* http_request,
                  u16_t http_request_len, int content_len, char* response_uri,
                  u16_t response_uri_len, u8_t* post_auto_wnd) override {
    (void)http_request;
    (void)http_request_len;
    (void)response_uri;
    (void)response_uri_len;
    (void)post_auto_wnd;

    if (!StrStartsWith(uri, kUrlPrefix)) return ERR_ARG;
    auto path = GetPath(uri + StrLen(kUrlPrefix) - 1);
    assert(!path.empty() && path.front() == '/');

    if (path.back() == '/') return ERR_ARG;
    assert(path.size() > 1);

    printf("POST %s (%d bytes) => %s\r\n", uri, content_len, path.c_str());

    auto dirname = LfsDirname(path.c_str());
    if (!LfsMakeDirs(dirname.c_str())) return ERR_ARG;

    auto file = std::make_unique<lfs_file_t>();
    if (lfs_file_open(Lfs(), file.get(), path.c_str(),
                      LFS_O_WRONLY | LFS_O_TRUNC | LFS_O_CREAT) < 0)
      return ERR_ARG;

    files_[connection] = {std::move(file), std::move(dirname)};
    return ERR_OK;
  }

  err_t PostReceiveData(void* connection, struct pbuf* p) override {
    auto* file = files_[connection].file.get();

    struct pbuf* cp = p;
    while (cp) {
      if (lfs_file_write(Lfs(), file, cp->payload, cp->len) != cp->len)
        return ERR_ARG;
      cp = p->next;
    }

    pbuf_free(p);
    return ERR_OK;
  }

  void PostFinished(void* connection, char* response_uri,
                    u16_t response_uri_len) override {
    const auto& entry = files_[connection];
    lfs_file_close(Lfs(), entry.file.get());
    snprintf(response_uri, response_uri_len, "%s%s", kUrlPrefix,
             entry.dirname.c_str() + 1);
    files_.erase(connection);
  }

 private:
  struct Entry {
    std::unique_ptr<lfs_file_t> file;
    std::string dirname;
  };
  std::map<void*, Entry> files_;  // connection-to-entry map
};

void Main() {
  printf("Web File Browser Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  FileHttpServer http_server;
  http_server.AddUriHandler(UriHandler);
  UseHttpServer(&http_server);

  std::string ip;
  if (GetUsbIpAddress(&ip)) {
    printf("BROWSE:   http://%s%s\r\n", ip.c_str(), kUrlPrefix);
    printf("UPLOAD:   curl -X POST http://%s%sfile --data-binary @file\r\n",
           ip.c_str(), kUrlPrefix);
    printf("DOWNLOAD: curl -O http://%s%sfile\r\n", ip.c_str(), kUrlPrefix);
  }
  vTaskSuspend(nullptr);
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
