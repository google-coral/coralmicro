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

#include "libs/base/http_server.h"

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "libs/base/strings.h"
#include "libs/base/utils.h"

// Hosts a simple HTTP server on the Dev Board Micro.
//
// Run it and open a web browser on your computer to the URL
// shown in the serial terminal, such as http://10.10.10.1/hello.html

namespace coralmicro {
namespace {

HttpServer::Content UriHandler(const char* path) {
  printf("Request received for %s\r\n", path);
  std::vector<uint8_t> html;
  html.reserve(64);
  if (std::strcmp(path, "/hello.html") == 0) {
    printf("Hello World!\r\n");
    StrAppend(&html, "<html><body>Hello World!</body></html>");
    return html;
  }
  return {};
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  printf("Starting server...\r\n");
  coralmicro::HttpServer http_server;
  http_server.AddUriHandler(coralmicro::UriHandler);
  coralmicro::UseHttpServer(&http_server);

  std::string ip;
  if (coralmicro::utils::GetUsbIpAddress(&ip)) {
    printf("GO TO:   http://%s/%s\r\n", ip.c_str(), "hello.html");
  }
  vTaskSuspend(nullptr);
}
