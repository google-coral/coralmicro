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

#ifndef SocketServer_h
#define SocketServer_h

#include <api/Server.h>

namespace coralmicro {
namespace arduino {

class SocketClient;

// This is not meant to be instantiated directly. Instead use
// `arduino::WiFiServer` or `arduino::EthernetServer`.
class SocketServer : public ::arduino::Server {
 public:

  // Initializes a new server socket.
  //
  // Each server socket can accept just one client connection.
  // @param The port number to use.
  SocketServer(int port) : port_(port) {}
  virtual ~SocketServer();

  // Starts the socket.
  void begin() override;

  // Writes some data to the socket.
  // @param c The data to write.
  // @return 1 if successful; -1 otherwise.
  size_t write(uint8_t c) override;

  // Writes an array of data to the socket.
  // @param array The array of data to write.
  // @param size The size of the array.
  // @return The size of the array written if successful; -1 otherwise.
  size_t write(const uint8_t* buf, size_t size) override;

  // Waits for and accepts a socket connection from a client.
  // This is a blocking call and returns only when a client has connected
  // to this server socket's port.
  //
  // Note: Only one client can connect at a time.
  //
  // @return The client object to read and write with the connected client.
  // Before using the returned client, check if the client is connected (check
  // if the object evaluates true or call `arduino::SocketClient::connected()`)
  // and check if it has data to read with `arduino::SocketClient::available()`.
  SocketClient available();

  using Print::write;

 private:
  int sock_;
  int port_;
};

}  // namespace arduino
}  // namespace coralmicro

#endif  // SocketServer_h
