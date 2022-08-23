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

class SocketClient : public ::arduino::Client {
 public:
  virtual ~SocketClient() { stop(); }
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
  int connect(IPAddress ip, uint16_t port) override;
  int connect(const char* host, uint16_t port) override;
  size_t write(uint8_t c) override;
  size_t write(const uint8_t* buf, size_t size) override;
  int available() override;
  int read() override;
  int read(uint8_t* buf, size_t size) override;
  int peek() override;
  void flush() override;
  void stop() override;
  uint8_t connected() override;
  operator bool() override;

 private:
  friend class SocketServer;
  void set_sock(int sock) { sock_ = sock; }
  int sock_ = -1;
};

}  // namespace arduino
}  // namespace coralmicro

#endif  // SocketClient_h