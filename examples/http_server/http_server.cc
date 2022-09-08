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

#include "libs/base/led.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"

// Hosts a simple HTTP server on the Dev Board Micro.
//
// NOTE: This HTTP server is accessible via USB only, which works from
// Windows and Linux only.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e http_server
//
// Then open a web browser on your computer connected via USB to the
// URL shown in the serial console, such as http://10.10.10.1/hello.html

// [start-snippet:http-server]
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

void Main() {
  printf("HTTP Server Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  printf("Starting server...\r\n");
  HttpServer http_server;
  http_server.AddUriHandler(UriHandler);
  UseHttpServer(&http_server);

  std::string ip;
  if (GetUsbIpAddress(&ip)) {
    printf("GO TO:   http://%s/%s\r\n", ip.c_str(), "hello.html");
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
// [end-snippet:http-server]
