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

#include "libs/base/led.h"
#include "libs/base/mutex.h"
#include "libs/base/network.h"
#include "libs/base/wifi.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/sockets.h"

// Creates a Wi-Fi socket server and prints all incoming text.
//
// For example, you can connect to the server socket from a new terminal on
// your computer with a utility such as netcat, passing the ip address and port:
//   nc 192.168.247.119 31337
// Then type into the terminal and it will be repeated in the board's serial
// console (also in the Arduino IDE Serial Monitor)
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e wifi_server

namespace coralmicro {
[[noreturn]] void Main() {
  printf("WiFi Server Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  printf("Attempting to use Wi-Fi...\r\n");
  // Uncomment me to use the external antenna.
  // SetWiFiAntenna(WiFiAntenna::kExternal);
  bool success = WiFiTurnOn(/*default_iface=*/true);
  if (!success) {
    printf("Failed to turn on Wi-Fi\r\n");
    vTaskSuspend(nullptr);
  }
  success = WiFiConnect();
  if (!success) {
    printf("Failed to connect to Wi-Fi\r\n");
    vTaskSuspend(nullptr);
  }
  printf("Wi-Fi connected\r\n");
  auto our_ip_addr = WiFiGetIp();

  if (our_ip_addr.has_value()) {
    printf("DHCP succeeded, our IP is %s.\r\n", our_ip_addr.value().c_str());
  } else {
    printf("We didn't get an IP via DHCP, not progressing further.\r\n");
    vTaskSuspend(nullptr);
  }

  const int server_socket = SocketServer(31337, 5);
  if (server_socket == -1) {
    printf("ERROR: Cannot start server.\r\n");
    vTaskSuspend(nullptr);
  }

  while (true) {
    const int client_socket = accept(server_socket, nullptr, nullptr);
    if (client_socket == -1) {
      printf("ERROR: Cannot connect client.\r\n");
      continue;
    }
    while (true) {
      if (int available = SocketAvailable(client_socket); available > 0) {
        std::vector<char> arr(available);
        ReadArray(client_socket, arr.data(), arr.size());
        printf("%s\r\n", arr.data());
      }
    }
  }
}
}  // namespace coralmicro

extern "C" [[noreturn]] void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
