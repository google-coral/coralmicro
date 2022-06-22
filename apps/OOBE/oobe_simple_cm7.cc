#include "libs/base/ipc_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

extern "C" void app_main(void* param) {
  (void)param;
  coral::micro::IPCM7::GetSingleton()->StartM4();
  vTaskSuspend(nullptr);
}
