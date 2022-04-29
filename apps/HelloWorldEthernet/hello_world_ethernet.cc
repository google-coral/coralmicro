#include "libs/base/ethernet.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/dns.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"
#include <cstdio>

/* clang-format off */
#include "libs/curl/curl.h"
/* clang-format on */

struct DnsCallbackArg {
    SemaphoreHandle_t sema;
    ip_addr_t ip_addr;
};

static size_t curl_writefunction(void* contents, size_t size, size_t nmemb, void* param) {
    size_t* bytes_curled = reinterpret_cast<size_t*>(param);
    *bytes_curled = *bytes_curled + (size * nmemb);
    return size * nmemb;
}

static void CURLRequest(const char *url) {
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
            printf("Curling of %s successful! (%d bytes curled)\r\n", url, bytes_curled);
        }
        curl_easy_cleanup(curl);
    }
}

extern "C" void app_main(void *param) {
    printf("Hello world Ethernet.\r\n");

    coral::micro::InitializeEthernet(true);

    struct netif *ethernet = coral::micro::GetEthernetInterface();

    printf("Waiting on DHCP...\r\n");
    while (true) {
        if (netif_is_up(ethernet)) {
            struct dhcp *dhcp = netif_dhcp_data(ethernet);
            if (dhcp->state == DHCP_STATE_BOUND) {
                break;
            }
        }
        taskYIELD();
    }
    printf("DHCP succeeded, now to try DNS!\r\n");

    DnsCallbackArg dns_arg;
    dns_arg.sema = xSemaphoreCreateBinary();
    LOCK_TCPIP_CORE();
    dns_gethostbyname("google.com", &dns_arg.ip_addr,
        [](const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
            DnsCallbackArg* dns_arg = reinterpret_cast<DnsCallbackArg*>(callback_arg);
            if (ipaddr) {
                memcpy(&dns_arg->ip_addr, ipaddr, sizeof(ipaddr));
            }
            xSemaphoreGive(dns_arg->sema);
        },
    &dns_arg);
    UNLOCK_TCPIP_CORE();
    xSemaphoreTake(dns_arg.sema, portMAX_DELAY);
    vSemaphoreDelete(dns_arg.sema);

    printf("google.com -> %s\r\n", ipaddr_ntoa(&dns_arg.ip_addr));

    curl_global_init(CURL_GLOBAL_ALL);
    CURLRequest("https://www.google.com");
    curl_global_cleanup();

    vTaskSuspend(NULL);
}
