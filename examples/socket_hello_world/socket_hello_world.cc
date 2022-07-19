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

#include <cstdio>

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

extern "C" void app_main(void* param) {
  printf("Hello socket.\r\n");

  int listening_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  setsockopt(listening_socket, 0, SO_RCVTIMEO, &tv, sizeof(tv));

  struct sockaddr_in bind_address;
  bind_address.sin_family = AF_INET;
  bind_address.sin_port = PP_HTONS(31337);
  bind_address.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

  bind(listening_socket, reinterpret_cast<struct sockaddr*>(&bind_address),
       sizeof(bind_address));
  listen(listening_socket, 1);

  const char* fixed_str = "Hello socket.\r\n";
  while (true) {
    int accepted_socket = accept(listening_socket, nullptr, nullptr);
    send(accepted_socket, fixed_str, strlen(fixed_str), 0);
    closesocket(accepted_socket);
  }

  vTaskSuspend(nullptr);
}
