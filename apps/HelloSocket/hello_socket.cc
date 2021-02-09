#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/FreeRTOS-Plus-TCP/include/FreeRTOS_IP.h"
#include "third_party/FreeRTOS-Plus-TCP/include/FreeRTOS_Sockets.h"

#include <cstdio>

extern "C" void app_main(void *param) {
    printf("Hello socket.\r\n");

    struct freertos_sockaddr xBindAddress, xClient;
    socklen_t xSize = sizeof(xClient);
    Socket_t xListeningSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

    TickType_t xReceiveTimeOut = portMAX_DELAY;
    FreeRTOS_setsockopt(xListeningSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof(xReceiveTimeOut));

    xBindAddress.sin_port = (uint16_t)31337;
    xBindAddress.sin_port = FreeRTOS_htons(xBindAddress.sin_port);
    FreeRTOS_bind(xListeningSocket, &xBindAddress, sizeof(xBindAddress));
    FreeRTOS_listen(xListeningSocket, 20);

    while (true) {
        const char *fixed_str = "Hello socket.\r\n";
        Socket_t xConnectedSocket = FreeRTOS_accept(xListeningSocket, &xClient, &xSize);
        FreeRTOS_send(xConnectedSocket, fixed_str, strlen(fixed_str), 0);
        FreeRTOS_closesocket(xConnectedSocket);
        taskYIELD();
    }
}
