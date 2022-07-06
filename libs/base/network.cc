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

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

namespace coralmicro {

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
    const uint8_t header[] = {static_cast<uint8_t>(size),
                              static_cast<uint8_t>(size >> 8),
                              static_cast<uint8_t>(size >> 16),
                              static_cast<uint8_t>(size >> 24), type};

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

    auto ret = lwip_bind(sockfd, reinterpret_cast<struct sockaddr*>(&bind_address),
                      sizeof(bind_address));
    if (ret == -1) return -1;

    ret = lwip_listen(sockfd, backlog);
    if (ret == -1) return -1;

    return sockfd;
}

int SocketAccept(int sockfd) { return lwip_accept(sockfd, nullptr, nullptr); }

void SocketClose(int sockfd) { lwip_close(sockfd); }

}  // namespace coralmicro
