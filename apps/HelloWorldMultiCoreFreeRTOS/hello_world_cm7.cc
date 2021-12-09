#include "libs/base/ipc_m7.h"
#include "libs/base/mutex.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>
#include <cstring>

extern "C" void app_main(void *param) {
    static_cast<valiant::IPCM7*>(valiant::IPC::GetSingleton())->StartM4();
    while (true) {
        valiant::MulticoreMutexLock lock(0);
        printf("[M7] Hello.\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
