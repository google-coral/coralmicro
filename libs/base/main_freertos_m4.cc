#include "libs/base/console_m4.h"
#include "libs/base/ipc.h"
#include "libs/base/tasks.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>
#include <cstring>

extern "C" void app_main(void *param);
extern "C" void BOARD_InitHardware();

extern "C" int main(int argc, char **argv) __attribute__((weak));
extern "C" int main(int argc, char **argv) {
    BOARD_InitHardware();
    valiant::IPC::GetSingleton()->Init();
    valiant::ConsoleInit();

    static StaticTask_t xTaskBuffer;
    static StackType_t xStack[configMINIMAL_STACK_SIZE * 10];
    xTaskCreateStatic(app_main, "app_main", configMINIMAL_STACK_SIZE * 10, NULL, APP_TASK_PRIORITY, xStack, &xTaskBuffer);

    vTaskStartScheduler();

    return 0;
}
