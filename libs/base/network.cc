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

#include "libs/base/network.h"

#include <errno.h>

#include <algorithm>
#include <cassert>
#include <cstring>

#include "libs/base/filesystem.h"
#include "libs/base/utils.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/api.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/dns.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

namespace coralmicro {

namespace {
inline constexpr const char kDnsServerPath[] = "/dns_server";
}

IOStatus ReadBytes(int fd, void* bytes, size_t size) {
  assert(fd >= 0);
  assert(bytes);

  char* buf = static_cast<char*>(bytes);
  while (size != 0) {
    auto ret = lwip_read(fd, buf, size);
    if (ret == 0) return IOStatus::kEof;
    if (ret == -1) {
      if (errno == EINTR) continue;
      return IOStatus::kError;
    }
    size -= ret;
    buf += ret;
  }
  return IOStatus::kOk;
}

IOStatus WriteBytes(int fd, const void* bytes, size_t size, size_t chunk_size) {
  assert(fd >= 0);
  assert(bytes);

  const char* buf = static_cast<const char*>(bytes);
  while (size != 0) {
    auto len = std::min(size, chunk_size);
    auto ret = lwip_write(fd, buf, len);
    if (ret == -1) {
      if (errno == EINTR) continue;
      return IOStatus::kError;
    }
    size -= len;
    buf += len;
  }

  return IOStatus::kOk;
}

IOStatus WriteMessage(int fd, uint8_t type, const void* bytes, size_t size,
                      size_t chunk_size) {
  const uint8_t header[] = {
      static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
      static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24), type};

  auto ret = WriteBytes(fd, header, sizeof(header), chunk_size);
  if (ret != IOStatus::kOk) return ret;

  return WriteBytes(fd, bytes, size, chunk_size);
}

bool SocketHasPendingInput(int sockfd) {
  char buf;
  return lwip_recv(sockfd, &buf, 1, MSG_DONTWAIT) == 1;
}

int SocketServer(int port, int backlog) {
  const int sockfd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1) return -1;

  struct sockaddr_in bind_address = {};
  bind_address.sin_family = AF_INET;
  bind_address.sin_port = PP_HTONS(port);
  bind_address.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

  auto ret =
      lwip_bind(sockfd, reinterpret_cast<struct sockaddr*>(&bind_address),
                sizeof(bind_address));
  if (ret == -1) return -1;

  ret = lwip_listen(sockfd, backlog);
  if (ret == -1) return -1;

  return sockfd;
}

int SocketClient(ip_addr_t ip, int port) {
  const int sockfd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1) return -1;

  struct sockaddr_in connect_addr = {};
  connect_addr.sin_len = sizeof(connect_addr);
  connect_addr.sin_family = AF_INET;
  connect_addr.sin_port = PP_HTONS(port);
  std::memcpy(&connect_addr.sin_addr, &ip.addr, sizeof(uint32_t));
  auto ret =
      lwip_connect(sockfd, reinterpret_cast<struct sockaddr*>(&connect_addr),
                   sizeof(connect_addr));
  if (ret == -1) return -1;

  return sockfd;
}

int SocketClient(const char* host, int port) {
  ip_addr_t lwip_addr;
  err_t err = netconn_gethostbyname(host, &lwip_addr);
  if (err != ERR_OK) {
    return -1;
  }
  return SocketClient(lwip_addr, port);
}

int SocketAccept(int sockfd) { return lwip_accept(sockfd, nullptr, nullptr); }

void SocketClose(int sockfd) { lwip_close(sockfd); }

int SocketAvailable(int sockfd) {
  int i;
  lwip_ioctl(sockfd, FIONREAD, &i);
  return i;
}

void DnsInit() {
  ip4_addr_t addr;
  if (!GetIpFromFile(kDnsServerPath, &addr)) return;
  dns_setserver(0, &addr);
}

ip4_addr_t DnsGetServer() {
  ip4_addr_t addr;
  memcpy(&addr, dns_getserver(0), sizeof(addr));
  return addr;
}

void DnsSetServer(ip4_addr_t addr, bool persist) {
  dns_setserver(0, &addr);
  if (persist) {
    std::string str;
    str.resize(IP4ADDR_STRLEN_MAX);
    if (ipaddr_ntoa_r(&addr, str.data(), IP4ADDR_STRLEN_MAX)) {
      LfsWriteFile(kDnsServerPath, str);
    }
  }
}

}  // namespace coralmicro
