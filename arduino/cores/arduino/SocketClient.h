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

#ifndef SocketClient_h
#define SocketClient_h

#include <api/Client.h>

#include <cassert>

namespace coralmicro {
namespace arduino {

// This is not meant to be instantiated directly. Instead use
// `arduino::WiFiClient` or `arduino::EthernetClient`.
class SocketClient : public ::arduino::Client {
 public:
  virtual ~SocketClient() { stop(); }
  // Initializes a new client socket.
  SocketClient() = default;
  SocketClient(const SocketClient& orig) = delete;
  SocketClient& operator=(const SocketClient& orig) = delete;
  SocketClient(SocketClient&& orig) {
    sock_ = orig.sock_;
    orig.sock_ = -1;
  }
  SocketClient& operator=(SocketClient&& orig) {
    if (this != &orig) {
      sock_ = orig.sock_;
      orig.sock_ = -1;
    }
    return *this;
  }

  // Start a socket connection with a server.
  // @param ip The server's ip address.
  // @param port The server's port number.
  // @return 1 if successful; -1 otherwise.
  int connect(IPAddress ip, uint16_t port) override;

  // Starts a socket connection with a server.
  // @param ip The server's hostname.
  // @param port The server's port number.
  // @return 1 if successful; -1 otherwise.
  int connect(const char* host, uint16_t port) override;

  // Writes some data to the socket.
  // @param c The data to write.
  // @return 1 if successful; -1 otherwise.
  size_t write(uint8_t c) override;

  // Writes an array of data to the socket.
  // @param array The array of data to write.
  // @param size The size of the array.
  // @return The size of the array written if successful; -1 otherwise.
  size_t write(const uint8_t* buf, size_t size) override;

  // Checks if the socket has bytes available to read.
  // @return The number of bytes available to read.
  int available() override;

  // Reads some data from the socket.
  // @return The data if successful; -1 otherwise.
  int read() override;

  // Reads an array of data from the socket.
  // @param The buffer in which to put the data.
  // @param The size of the buffer.
  // @return The size of the array read if successful; -1 otherwise.
  int read(uint8_t* buf, size_t size) override;

  // @cond Do not generate docs
  int peek() override;

  void flush() override;
  // @endcond

  // Close the socket.
  void stop() override;

  // Checks if the socket is connected.
  // @return True if connected; false otherwise.
  uint8_t connected() override;

  // True if the socket is connected; false otherwise.
  operator bool() override;

 private:
  friend class SocketServer;
  void set_sock(int sock) { sock_ = sock; }
  int sock_ = -1;
};

}  // namespace arduino
}  // namespace coralmicro

#endif  // SocketClient_h
