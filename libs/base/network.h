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

#ifndef LIBS_BASE_NETWORK_H_
#define LIBS_BASE_NETWORK_H_

#include <cstddef>
#include <cstdint>

namespace coral::micro {

enum class IOStatus { kOk, kEof, kError };

IOStatus ReadBytes(int fd, void* bytes, size_t size);

template <typename T>
IOStatus ReadArray(int fd, T* array, size_t array_size) {
    return ReadBytes(fd, array, array_size * sizeof(T));
}

IOStatus WriteBytes(int fd, const void* bytes, size_t size,
                    size_t chunk_size = 1024);

template <typename T>
IOStatus WriteArray(int fd, const T* array, size_t array_size) {
    return WriteBytes(fd, array, array_size * sizeof(T));
}

IOStatus WriteMessage(int fd, uint8_t type, const void* bytes, size_t size,
                      size_t chunk_size = 1024);

bool SocketHasPendingInput(int sockfd);

int SocketServer(int port, int backlog);

int SocketAccept(int sockfd);

void SocketClose(int sockfd);

}  // namespace coral::micro

#endif  // LIBS_BASE_NETWORK_H_
