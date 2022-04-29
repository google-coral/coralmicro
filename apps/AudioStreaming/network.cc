#include "apps/AudioStreaming/network.h"

#include <errno.h>

#include <algorithm>
#include <cassert>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

namespace coral::micro {

IOStatus ReadBytes(int fd, void* bytes, size_t size) {
    assert(fd >= 0);
    assert(bytes);

    char* buf = reinterpret_cast<char*>(bytes);
    while (size != 0) {
        auto ret = ::read(fd, buf, size);
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

    const char* buf = reinterpret_cast<const char*>(bytes);
    while (size != 0) {
        auto len = std::min(size, chunk_size);
        auto ret = ::write(fd, buf, len);
        if (ret == -1) {
            if (errno == EINTR) continue;
            return IOStatus::kError;
        }
        size -= len;
        buf += len;
    }

    return IOStatus::kOk;
}

int SocketServer(int port, int backlog) {
    const int s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1) {
        printf("ERROR: Cannot create server socket\n\r");
        return -1;
    }

    struct sockaddr_in bind_address;
    bind_address.sin_family = AF_INET;
    bind_address.sin_port = PP_HTONS(port);
    bind_address.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    auto ret = ::bind(s, reinterpret_cast<struct sockaddr*>(&bind_address),
                      sizeof(bind_address));
    if (ret == -1) {
        printf("ERROR: Cannot bind server socket\n\r");
        return -1;
    }

    ret = ::listen(s, backlog);
    if (ret == -1) {
        printf("ERROR: Cannot listen server socket\n\r");
        return -1;
    }

    return s;
}

}  // namespace coral::micro
