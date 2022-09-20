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

#ifndef LIBS_BASE_NETWORK_H_
#define LIBS_BASE_NETWORK_H_

#include <cstddef>
#include <cstdint>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/ip_addr.h"

namespace coralmicro {

// Socket I/O status response codes.
enum class IOStatus {
  // Operation succeeded.
  kOk,
  // Reached end-of-file during read operation.
  kEof,
  // An error occurred during operation.
  kError
};

// Reads data from a socket file descriptor to a buffer.
//
// @param fd The file descriptor to read from.
// @param bytes The buffer to output the data.
// @param size The number of bytes to read.
// @return The status result of the operation.
IOStatus ReadBytes(int fd, void* bytes, size_t size);

// Read an array of data from a socket file descriptor.
//
// @param fd The file descriptor to read from.
// @param array The array to output the data to.
// @param array_size The size of the array.
// @tparam T The data type of the array.
// @return The status result of the operation.
template <typename T>
IOStatus ReadArray(int fd, T* array, size_t array_size) {
  return ReadBytes(fd, array, array_size * sizeof(T));
}

// Writes data from a buffer into a socket file descriptor.
//
// @param fd The file descriptor to write to.
// @param bytes The buffer of data to write.
// @param size The size of the buffer.
// @param chunk_size The size of the chunk to write.
// @return The status result of the operation.
IOStatus WriteBytes(int fd, const void* bytes, size_t size,
                    size_t chunk_size = 1024);

// Writes data from an array into a socket file descriptor.
//
// @param fd The file descriptor to write to.
// @param array The array of data to write.
// @param size The size of the array.
// @param chunk_size The size of the chunk to write.
// @tparam T The data type of the array.
// @return The status result of the operation.
template <typename T>
IOStatus WriteArray(int fd, const T* array, size_t array_size) {
  return WriteBytes(fd, array, array_size * sizeof(T));
}

// Writes a `message` with custom type from a buffer into a socket file
// descriptor.
//
// The `message` is going to have a 40 bits prefix that contains 32 bits for the
// size and 8 bits for the custom message type, following by the bytes.
// @param fd The file descriptor to write to.
// @param type The type of the message. This parameter can be used to create
// custom message type.
// @param bytes The buffer of data to write.
// @param size The size of the buffer.
// @param chunk_size The size of the chunk to write.
// @return The status result of the operation.
IOStatus WriteMessage(int fd, uint8_t type, const void* bytes, size_t size,
                      size_t chunk_size = 1024);

// Checks whether a socket file descriptor still has some bytes to read.
//
// @param sockfd The socket file descriptor to check.
// @return True if there are bytes remaining to read; false otherwise.
bool SocketHasPendingInput(int sockfd);

// Starts a new TCP socket server.
//
// @param port The port to listen on.
// @param backlog The maximum length to which the queue of pending connections
// for sockfd may grow
// @return The server's socket file descriptor.
int SocketServer(int port, int backlog);

// Accepts a connection request from a listening socket.
//
// @param sockfd The listening socket file descriptor.
// @return The client's socket file descriptor.
int SocketAccept(int sockfd);

// Close a socket.
//
// @param sockfd The socket file descriptor to close.
void SocketClose(int sockfd);

// Starts a client-side connection with a server.
//
// @param ip The server's ip address.
// @param port The server's port number.
// @return The server's socket file descriptor.
int SocketClient(ip_addr_t ip, int port);

// Starts a client-side connection with a server.
//
// @param host The server's hostname.
// @param port The server's port number.
// @return The server's socket file descriptor.
int SocketClient(const char* host, int port);

// Checks if a socket has available bytes to read.
//
// @param sockfd The socket file descriptor to check.
// @return Number of bytes that can be read from the socket.
int SocketAvailable(int sockfd);

// @cond Do not generate docs
void DnsInit();
// @endcond

// Retrieves the configured DNS server for the network.
//
// @returns `ip4_addr_t` containing the IP address where DNS requests are sent.
ip4_addr_t DnsGetServer();

// Sets the DNS server for the network, and optionally persists it to flash
// memory.
//
// @param addr The address for the DNS server.
// @param persist True to persist the address to flash; false otherwise.
void DnsSetServer(ip4_addr_t addr, bool persist);

}  // namespace coralmicro

#endif  // LIBS_BASE_NETWORK_H_
