#include "libs/base/console_m7.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/network.h"
#include "libs/base/random.h"
#include "libs/base/tasks_m7.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/FreeRTOS-Plus-TCP/include/FreeRTOS_IP.h"

static const uint8_t ucIPAddress[4] = { 10, 10, 10, 200 };
static const uint8_t ucNetMask[4] = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[4] = { 10, 10, 10, 1 };
static const uint8_t ucDNSServerAddress[4] = { 8, 8, 8, 8 };
static uint8_t ucMACAddress[6] = { 0x00, 0x1A, 0x11, 0xBA, 0xDF, 0xAD };

extern "C" void app_main(void *param);
extern "C" void BOARD_InitHardware();

extern "C" int main(int argc, char **argv) __attribute__((weak));
extern "C" int main(int argc, char **argv) {
    BOARD_InitHardware();
    valiant::Random::GetSingleton()->Init();
    valiant::ConsoleM7::GetSingleton()->Init();
    valiant::IPCInit();
    valiant::InitializeNetworkEEM();
    valiant::UsbDeviceTask::GetSingleton()->Init();

    FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);

    xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE * 10, NULL, APP_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    return 0;
}
