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

// Hosts a simple HTTP server on the Dev Board Micro.
//
// NOTE: This HTTP server is accessible via USB only, which works from
// Windows and Linux only.
//
// After you upload the sketch, open a web browser on your computer connected
// via USB to the URL shown in the serial console, such as
// http://10.10.10.1/hello.html

#include "Arduino.h"
#include "libs/base/http_server.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"

namespace {
coralmicro::HttpServer http_server;
}
coralmicro::HttpServer::Content UriHandler(const char* path) {
  Serial.print("Request received for ");
  Serial.println(path);
  std::vector<uint8_t> html;
  html.reserve(64);
  if (std::strcmp(path, "/hello.html") == 0) {
    Serial.println("Hello World!");
    coralmicro::StrAppend(&html, "<html><body>Hello World!</body></html>");
    return html;
  }
  return {};
}

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino HTTP Server Example!");

  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  Serial.println("Starting server...");
  http_server.AddUriHandler(UriHandler);
  coralmicro::UseHttpServer(&http_server);

  std::string ip;
  if (coralmicro::GetUsbIpAddress(&ip)) {
    Serial.print("GO TO:   http://");
    Serial.print(ip.c_str());
    Serial.print("/");
    Serial.println("hello.html");
  }
}

void loop() {}
