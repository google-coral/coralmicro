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
class SocketServer : public ::arduino::Server {
 public:
  SocketServer(int port) : port_(port) {}
  virtual ~SocketServer();

  void begin() override;
  size_t write(uint8_t c) override;
  size_t write(const uint8_t* buf, size_t size) override;
  SocketClient available();

  using Print::write;

 private:
  int sock_;
  int port_;
};

}  // namespace arduino
}  // namespace coralmicro

#endif  // SocketServer_h