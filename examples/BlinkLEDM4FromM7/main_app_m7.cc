#include "libs/base/ipc_m7.h"
#include "libs/base/mutex.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include <cstdio>


extern "C" [[noreturn]] void app_main(void *param) {
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    printf("Starting M4...\r\n");
    valiant::IPCM7::GetSingleton()->StartM4();
    vTaskSuspend(nullptr);
}
