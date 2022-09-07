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

namespace coralmicro {

// Defines an HTTP server on the device.
//
// This is a light wrapper around the lwIP stack. For more detail,
// see https://www.nongnu.org/lwip/2_1_x/index.html.
//
// For an example, see examples/http_server/http_server.cc.
class HttpServer {
 public:
  virtual ~HttpServer() = default;

  // Called when an HTTP POST request is first received.
  // This must be implemented by subclasses that want to handle posts.
  //
  // @param connection Unique connection identifier, valid until
  // `PostFinished()` is called.
  // @param uri The HTTP header URI receiving the POST request.
  // @param http_request  The raw HTTP request (the first packet, normally).
  // @param http_request_len  Size of 'http_request'.
  // @param content_len Content-Length from HTTP header.
  // @param response_uri Filename of response file, to be filled when denying
  //   the request.
  // @param response_uri_len Size of the 'response_uri' buffer.
  // @param post_auto_wnd Set this to 0 to let the callback code handle window
  //   updates by calling `httpd_post_data_recved` (to throttle rx speed)
  //   default is 1 (httpd handles window updates automatically)
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

  // Called for each packet buffer of data that is received for a POST.
  //
  // @param connection  Unique connection identifier.
  // @param p  Received data as a
  // [pbuf](https://www.nongnu.org/lwip/2_1_x/structpbuf.html).
  // **ATTENTION:** Your application is responsible for freeing the pbufs!
  virtual err_t PostReceiveData(void* connection, struct pbuf* p) {
    (void)connection;
    (void)p;
    return ERR_ARG;
  };

  // Called when all data is received or when the connection is closed. The
  // application must return the filename/URI of a file to send in response to
  // this POST request. If the response_uri buffer is untouched, a 404 response
  // is returned.
  //
  // @param connection Unique connection identifier.
  // @param response_uri Filename of response file, to be filled when denying
  //   the request.
  // @param response_uri_len Size of the 'response_uri' buffer.
  virtual void PostFinished(void* connection, char* response_uri,
                            u16_t response_uri_len) {
    (void)connection;
    (void)response_uri;
    (void)response_uri_len;
  };

  // Called once to handle CGI for every URI with parameters.
  //
  // @param file The file received.
  // @param uri The HTTP header URI.
  // @param iNumParams The number of parameters in the URI.
  // @param pcParam Parameter names from the URI.
  // @param pcValue Values for each parameter.
  // file, uri, count, http_cgi_params, http_cgi_param_vals
  virtual void CgiHandler(struct fs_file* file, const char* uri, int iNumParams,
                          char** pcParam, char** pcValue) {
    (void)file;
    (void)uri;
    (void)iNumParams;
    (void)pcParam;
    (void)pcValue;
  };

  // Called first for every opened file to allow opening custom files
  // that are not included in fsdata(_custom).c.
  virtual int FsOpenCustom(struct fs_file* file, const char* name);

  // Called to read custom files.
  virtual int FsReadCustom(struct fs_file* file, char* buffer, int count);

  // Called to close custom files.
  virtual void FsCloseCustom(struct fs_file* file);

 public:
  // Defines a static buffer in which to return data from the server
  struct StaticBuffer {
    // A pointer to the data buffer
    const uint8_t* buffer;
    // The size of the buffer in bytes
    size_t size;
  };

  // Defines the allowed response types returned by `AddUriHandler()`.
  // Successful requests will typically respond with the content in
  // a string, a dynamic buffer (a vector), or a `StaticBuffer`, or an
  // empty vector if the URI is unhandled.
  using Content = std::variant<std::monostate,        // Not found
                               std::string,           // Filename
                               std::vector<uint8_t>,  // Dynamic buffer
                               StaticBuffer>;         // Static buffer

  // Represents the callback function type required by `AddUriHandler()`.
  using UriHandler = std::function<Content(const char* uri)>;

  // Adds a URI handler function for the server.
  //
  // You can specify multiple handlers and incoming requests will be sent to
  // the handlers in the order that each handler was added with this function.
  // That is, the first handler you add receives all requests first, and if
  // it does not handle it, then it is sent to the next handler, and so on.
  //
  // @param handler A callback function to handle each incoming HTTP
  //   request. It must accept the URI path as a char string and return
  //   the response as `Content`.
  void AddUriHandler(UriHandler handler) {
    uri_handlers_.push_back(std::move(handler));
  }

 private:
  std::vector<UriHandler> uri_handlers_;
};

// Starts an HTTP server.
//
// To handle server requests, you must pass your implementation of
// `UriHandler()` to `AddUriHandler()`.
// @param The server to start.
void UseHttpServer(HttpServer* server);

}  // namespace coralmicro

#endif  // LIBS_BASE_HTTP_SERVER_H_
