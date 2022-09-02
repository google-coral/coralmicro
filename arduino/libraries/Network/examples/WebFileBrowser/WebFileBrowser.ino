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

#include <coralmicro_SD.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

#include "Arduino.h"
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
using namespace coralmicro;
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

bool DirHtml(SDFile dir, const char* dirname, std::vector<uint8_t>* html) {
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
  if (!SD.exists(dir.name())) return false;
  while (true) {
    SDFile entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    if (!entry.isDirectory()) {
      StrAppend(html, "<tr>\r\n");
      StrAppend(html, "<td>&#128462;</td>\r\n");
      StrAppend(html, "<td><a href=\"%s%s%s\">%s</a></td>\r\n", kUrlPrefix,
                dirname + 1, entry.name(), entry.name());
      StrAppend(html, "<td>%d</td>", entry.size());
      StrAppend(html, "</tr>\r\n");
    } else {
      if (std::strcmp(entry.name(), ".") == 0) continue;
      StrAppend(html, "<tr>\r\n");
      StrAppend(html, "<td>&#128193;</td>\r\n");
      StrAppend(html, "<td><a href=\"%s%s%s/\">%s/</a></td>\r\n", kUrlPrefix,
                dirname + 1, entry.name(), entry.name());
      StrAppend(html, "<td></td>\r\n");
      StrAppend(html, "</tr>\r\n");
    }
    entry.close();
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

  Serial.print("GET ");
  Serial.print(uri);
  Serial.print(" => ");
  Serial.println(path.c_str());

  if (SD.exists(path.c_str())) {
    SDFile file = SD.open(path.c_str());
    if (!file.isDirectory()) {
      file.close();
      return path;
    } else {
      // Make sure directory path has '/' at the end.
      if (path.back() != '/') path.push_back('/');

      std::vector<uint8_t> html;
      html.reserve(1024);
      if (!DirHtml(file, path.c_str(), &html)) {
        file.close();
        return {};
      }
      file.close();
      return html;
    }
  }
  return {};
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

    Serial.print("POST ");
    Serial.print(uri);
    Serial.print(" (");
    Serial.print(content_len);
    Serial.print(" bytes) => ");
    Serial.println(path.c_str());

    auto dirname = LfsDirname(path.c_str());
    if (!SD.mkdir(dirname.c_str())) return ERR_ARG;

    SD.remove(path.c_str());
    SDFile file = SD.open(path.c_str(), FILE_WRITE);
    if (!file) return ERR_ARG;

    files_[connection] = {std::move(file), std::move(dirname)};
    return ERR_OK;
  }

  err_t PostReceiveData(void* connection, struct pbuf* p) override {
    SDFile file = files_[connection].file;
    struct pbuf* cp = p;
    while (cp) {
      if (file.write(reinterpret_cast<uint8_t const*>(cp->payload), cp->len) !=
          cp->len)
        return ERR_ARG;
      cp = p->next;
    }
    pbuf_free(p);
    return ERR_OK;
  }

  void PostFinished(void* connection, char* response_uri,
                    u16_t response_uri_len) override {
    auto& entry = files_[connection];
    entry.file.close();
    snprintf(response_uri, response_uri_len, "%s%s", kUrlPrefix,
             entry.dirname.c_str() + 1);
    files_.erase(connection);
  }

 private:
  struct Entry {
    SDFile file;
    std::string dirname;
  };
  std::map<void*, Entry> files_;  // connection-to-entry map
};

FileHttpServer http_server;
}  // namespace

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Web File Browser Example!");

  SD.begin();

  http_server.AddUriHandler(UriHandler);
  UseHttpServer(&http_server);

  std::string ip;
  if (GetUsbIpAddress(&ip)) {
    Serial.print("BROWSE:   http://");
    Serial.print(ip.c_str());
    Serial.println(kUrlPrefix);
    Serial.print("UPLOAD:   curl -X POST http://");
    Serial.print(ip.c_str());
    Serial.print(kUrlPrefix);
    Serial.println("file --data-binary @file");
    Serial.print("DOWNLOAD: curl -O http://");
    Serial.print(ip.c_str());
    Serial.print(kUrlPrefix);
    Serial.println("file");
  }
}

void loop() { delay(1000); }
