#include "libs/base/ethernet.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/dns.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"
#include <cstdio>

struct DnsCallbackArg {
    SemaphoreHandle_t sema;
    ip_addr_t ip_addr;
};

extern "C" void app_main(void *param) {
    printf("Hello world Ethernet.\r\n");

    valiant::InitializeEthernet();

    struct netif *ethernet = valiant::GetEthernetInterface();

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

    vTaskSuspend(NULL);
}
