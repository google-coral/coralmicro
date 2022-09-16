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

#include "SocketClient.h"

#include <errno.h>

#include <cstring>

#include "libs/base/network.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/api.h"

namespace coralmicro {
namespace arduino {

int SocketClient::connect(IPAddress ip, uint16_t port) {
  // Socket already exists.
  if (sock_ >= 0) {
    return -1;
  }

  ip_addr_t addr;
  std::memcpy(&addr.addr, &ip[0], sizeof(uint32_t));
  sock_ = coralmicro::SocketClient(addr, port);
  if (sock_ < 0) {
    return -1;
  }

  return 1;
}

int SocketClient::connect(const char *host, uint16_t port) {
  // Socket already exists.
  if (sock_ >= 0) {
    return -1;
  }

  sock_ = coralmicro::SocketClient(host, port);
  if (sock_ < 0) {
    return -1;
  }

  return 1;
}

size_t SocketClient::write(uint8_t c) { return this->write(&c, 1); }

size_t SocketClient::write(const uint8_t *buf, size_t size) {
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

int SocketClient::available() {
  if (sock_ < 0) {
    return 0;
  }
  return coralmicro::SocketAvailable(sock_);
}

int SocketClient::read() {
  uint8_t buf;
  int ret = this->read(&buf, 1);
  if (ret < 0) {
    return -1;
  }
  return buf;
}

int SocketClient::read(uint8_t *buf, size_t size) {
  if (sock_ < 0) {
    return -1;
  }

  auto ret = coralmicro::ReadArray(sock_, buf, size);
  if (ret != IOStatus::kOk) {
    if (errno == EIO || errno == ENOTCONN) {
      sock_ = -1;
    }
    return -1;
  }
  return size;
}

int SocketClient::peek() { return -1; }

void SocketClient::flush(){};

void SocketClient::stop() {
  if (sock_ >= 0) {
    coralmicro::SocketClose(sock_);
    sock_ = -1;
  }
}

uint8_t SocketClient::connected() { return sock_ >= 0; }

SocketClient::operator bool() { return connected(); }

}  // namespace arduino
}  // namespace coralmicro
