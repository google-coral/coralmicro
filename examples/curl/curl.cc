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
#include <memory>

#if defined(CURL_ETHERNET)
#include "libs/base/ethernet.h"
#elif defined(CURL_WIFI)
#include "libs/base/wifi.h"
#endif

#include "libs/a71ch/a71ch.h"
#include "libs/base/check.h"
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/strings.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/dns.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/tcpip.h"

/* clang-format off */
#include "libs/curl/curl.h"
/* clang-format on */

// Demonstrates how to connect to the internet using either Wi-Fi or ethernet.
// Be sure you have either the Coral Wireless Add-on Board or PoE Add-on Board
// connected.
//
// When flashing this example, you must specify the "subapp" module name for
// the flashtool, which you can see in the local CMakeLists.txt file.
// For example, here's how to flash the Wi-Fi version of this example:
/*
python3 scripts/flashtool.py -e curl --subapp curl_wifi \
        --wifi_ssid network-name --wifi_psk network-password
*/

namespace coralmicro {
namespace {
struct DnsCallbackArg {
  SemaphoreHandle_t sema;
  const char* hostname;
  ip_addr_t* ip_addr;
};

size_t CurlWrite(void* contents, size_t size, size_t nmemb, void* param) {
  auto* bytes_curled = static_cast<size_t*>(param);
  *bytes_curled += size * nmemb;
  return size * nmemb;
}

void CurlRequest(const char* url) {
  CURL* curl = curl_easy_init();
  printf("Curling %s\r\n", url);
  if (curl) {
    size_t bytes_curled = 0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bytes_curled);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      printf("curl_easy_perform failed: %s\r\n", curl_easy_strerror(res));
    } else {
      printf("Curling of %s successful! (%d bytes curled)\r\n", url,
             bytes_curled);
    }
    curl_easy_cleanup(curl);
  }
}

void PerformDnsLookup(const char* hostname, ip_addr_t* addr) {
  DnsCallbackArg arg{};
  arg.ip_addr = addr;
  arg.sema = xSemaphoreCreateBinary();
  arg.hostname = hostname;
  CHECK(arg.sema);
  tcpip_callback(
      [](void* ctx) {
        auto* arg = static_cast<DnsCallbackArg*>(ctx);
        dns_gethostbyname(
            arg->hostname, arg->ip_addr,
            [](const char* name, const ip_addr_t* ipaddr, void* callback_arg) {
              auto* arg = static_cast<DnsCallbackArg*>(callback_arg);
              if (ipaddr != nullptr) {
                ip4_addr_copy(*arg->ip_addr, *ipaddr);
              }
              CHECK(xSemaphoreGive(arg->sema) == pdTRUE);
            },
            arg);
      },
      &arg);
  CHECK(xSemaphoreTake(arg.sema, portMAX_DELAY) == pdTRUE);
  vSemaphoreDelete(arg.sema);
}

void Main() {
  printf("Camera Curl Example!\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

  std::optional<std::string> our_ip_addr;
#if defined(CURL_ETHERNET)
  printf("Attempting to use ethernet...\r\n");
  CHECK(EthernetInit(/*default_iface=*/true));
  our_ip_addr = EthernetGetIp();
#elif defined(CURL_WIFI)
  printf("Attempting to use Wi-Fi...\r\n");
  // Uncomment me to use the external antenna.
  // SetWiFiAntenna(WiFiAntenna::kExternal);
  bool success = WiFiTurnOn(/*default_iface=*/true);
  if (!success) {
    printf("Failed to turn on Wi-Fi\r\n");
    return;
  }
  success = WiFiConnect();
  if (!success) {
    printf("Failed to connect to Wi-Fi\r\n");
    return;
  }
  printf("Wi-Fi connected\r\n");
  our_ip_addr = WiFiGetIp();
#endif

  if (our_ip_addr.has_value()) {
    printf("DHCP succeeded, our IP is %s.\r\n", our_ip_addr.value().c_str());
  } else {
    printf("We didn't get an IP via DHCP, not progressing further.\r\n");
    return;
  }

  // Wait for the clock to be set via NTP.
  struct timeval tv;
  int gettimeofday_retries = 10;
  do {
    if (gettimeofday(&tv, nullptr) == 0) {
      break;
    }
    gettimeofday_retries--;
    vTaskDelay(pdMS_TO_TICKS(1000));
  } while (gettimeofday_retries);
  if (!gettimeofday_retries) {
    printf("Clock was never set via NTP.\r\n");
    return;
  }

  // Initialize A71CH to provide entropy for SSL.
  CHECK(A71ChInit());

  const char* hostname = "www.example.com";
  ip_addr_t dns_ip_addr;
  ip4_addr_set_any(&dns_ip_addr);
  PerformDnsLookup(hostname, &dns_ip_addr);
  if (!ip4_addr_isany(&dns_ip_addr)) {
    printf("%s -> %s\r\n", hostname, ipaddr_ntoa(&dns_ip_addr));
  } else {
    printf("Cannot resolve %s host name\r\n", hostname);
    return;
  }

  curl_global_init(CURL_GLOBAL_ALL);
  constexpr const char* protocols[] = {"http", "https"};
  for (auto proto : protocols) {
    std::string uri;
    StrAppend(&uri, "%s://%s/", proto, hostname);
    CurlRequest(uri.c_str());
  }
  curl_global_cleanup();
  printf("Done.\r\n");
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
