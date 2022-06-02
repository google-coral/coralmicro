#include <cstdio>

#include "libs/base/ipc_m7.h"
#include "libs/base/mutex.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"

extern "C" [[noreturn]] void app_main(void* param) {
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    printf("Starting M4...\r\n");
    coral::micro::IPCM7::GetSingleton()->StartM4();
    vTaskSuspend(nullptr);
}
