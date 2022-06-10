#include <cstdio>

#include "libs/base/ipc_m7.h"
#include "libs/base/mutex.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"

// Does nothing except start the M4, which runs blink_led_m4

extern "C" [[noreturn]] void app_main(void* param) {
    printf("Starting M4 from M7...\r\n");
    coral::micro::IPCM7::GetSingleton()->StartM4();
    vTaskSuspend(nullptr);
}
