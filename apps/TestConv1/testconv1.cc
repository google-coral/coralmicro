#include "third_party/freertos_kernel/include/projdefs.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testconv1/testconv1.h"
#include "libs/tpu/edgetpu_manager.h"

extern "C" void app_main(void *param) {
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();

    if (!valiant::testconv1::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(NULL);
    }

    bool run = true;
    size_t counter = 0;
    while (true) {
        if (run) {
            run = valiant::testconv1::loop();
            ++counter;
            if ((counter % 100) == 0) {
                printf("Execution %u...\r\n", counter);
            }
        }
    }
    vTaskSuspend(NULL);
}
