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

#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/dns.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/tcpip.h"

/* clang-format off */
#include "libs/curl/curl.h"
/* clang-format on */

namespace coralmicro {

namespace {
struct DnsCallbackArg {
    SemaphoreHandle_t sema;
    const char *hostname;
    ip_addr_t ip_addr;
};

static size_t curl_writefunction(void* contents, size_t size, size_t nmemb,
                                 void* param) {
    size_t* bytes_curled = reinterpret_cast<size_t*>(param);
    *bytes_curled = *bytes_curled + (size * nmemb);
    return size * nmemb;
}

static void CURLRequest(const char* url) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    printf("Curling %s\r\n", url);
    if (curl) {
        size_t bytes_curled = 0;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bytes_curled);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writefunction);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("curl_easy_perform failed: %s\r\n", curl_easy_strerror(res));
        } else {
            printf("Curling of %s successful! (%d bytes curled)\r\n", url,
                   bytes_curled);
        }
        curl_easy_cleanup(curl);
    }
}

bool PerformDnsLookup(const char* hostname, ip_addr_t* addr) {
    DnsCallbackArg dns_arg;
    dns_arg.sema = xSemaphoreCreateBinary();
    dns_arg.hostname = hostname;
    tcpip_callback([](void* ctx) {
        DnsCallbackArg* dns_arg = static_cast<DnsCallbackArg*>(ctx);
        dns_gethostbyname(
            dns_arg->hostname, &dns_arg->ip_addr,
            [](const char* name, const ip_addr_t* ipaddr, void* callback_arg) {
                DnsCallbackArg* dns_arg =
                    reinterpret_cast<DnsCallbackArg*>(callback_arg);
                if (ipaddr) {
                    memcpy(&dns_arg->ip_addr, ipaddr, sizeof(ipaddr));
                }
                xSemaphoreGive(dns_arg->sema);
            },
            dns_arg);
    }, &dns_arg);
    xSemaphoreTake(dns_arg.sema, portMAX_DELAY);
    vSemaphoreDelete(dns_arg.sema);
    return true;
}

}  // namespace
void Main() {
    std::optional<std::string> our_ip_addr = std::nullopt;
#if defined(CURL_ETHERNET)
    coralmicro::InitializeEthernet(true);
    our_ip_addr = coralmicro::GetEthernetIp();
#elif defined(CURL_WIFI)
    // Uncomment me to use the external antenna.
    // coralmicro::SetWiFiAntenna(coralmicro::WiFiAntenna::kExternal);
    bool success = false;
    success = coralmicro::WiFiTurnOn();
    if (!success) {
        printf("Failed to turn on Wifi\r\n");
        return;
    }
    success = coralmicro::WiFiConnect();
    if (!success) {
        printf("falied to connect wifi\r\n");
        return;
    }
    our_ip_addr = coralmicro::WiFiGetIp();
#endif

    if (our_ip_addr.has_value()) {
        printf("DHCP succeeded, our IP is %s.\r\n", our_ip_addr.value().c_str());
    } else {
        printf("We didn't get an IP via DHCP, not progressing further.\r\n");
        return;
    }


    const char *hostname = "www.example.com";
    ip_addr_t dns_ip_addr;
    PerformDnsLookup(hostname, &dns_ip_addr);
    printf("%s -> %s\r\n", hostname, ipaddr_ntoa(&dns_ip_addr));

    curl_global_init(CURL_GLOBAL_ALL);
    const char *uri_fmt = "http://%s:80/";
    int size = snprintf(nullptr, 0, uri_fmt, hostname);
    auto uri = std::make_unique<char>(size + 1);
    snprintf(uri.get(), size + 1, uri_fmt, hostname);
    CURLRequest(uri.get());
    curl_global_cleanup();

    vTaskSuspend(nullptr);
}
}  // namespace coralmicro
extern "C" void app_main(void* param) {
    (void)param;
    coralmicro::Main();
    vTaskSuspend(nullptr);
}
