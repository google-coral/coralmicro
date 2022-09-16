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

#include "SocketServer.h"

#include <errno.h>

#include <utility>

#include "SocketClient.h"
#include "libs/base/network.h"

namespace coralmicro {
namespace arduino {

SocketServer::~SocketServer() {
  if (sock_) {
    coralmicro::SocketClose(sock_);
    sock_ = -1;
  }
}

void SocketServer::begin() {
  sock_ = coralmicro::SocketServer(port_, 1);
  if (sock_ < 0) {
    assert(false);
  }
}

size_t SocketServer::write(uint8_t c) { return this->write(&c, 1); }

size_t SocketServer::write(const uint8_t* buf, size_t size) {
  if (sock_ < 0) {
    return -1;
  }
  auto ret = coralmicro::WriteArray(sock_, buf, size);
  if (ret != IOStatus::kOk) {
    if (errno == EIO || errno == ENOTCONN) {
      sock_ = -1;
    }
    return -1;
  }
  return size;
}

SocketClient SocketServer::available() {
  SocketClient client;
  if (sock_ < 0) {
    return client;
  }
  int client_socket = coralmicro::SocketAccept(sock_);
  if (client_socket < 0) {
    return client;
  }
  client.set_sock(client_socket);
  return std::move(client);
}

}  // namespace arduino
}  // namespace coralmicro
