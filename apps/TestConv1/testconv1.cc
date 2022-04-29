#include "third_party/freertos_kernel/include/projdefs.h"
#include "libs/base/gpio.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testconv1/testconv1.h"
#include "libs/tpu/edgetpu_manager.h"

extern "C" void app_main(void *param) {
    if (!coral::micro::testconv1::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(NULL);
    }

    coral::micro::gpio::SetGpio(coral::micro::gpio::kTpuLED, true);

    size_t counter = 0;
    while (true) {
        coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
        coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();

        bool run = true;
        for (int i = 0; i < 1000; ++i) {
            if (run) {
                run = coral::micro::testconv1::loop();
                ++counter;
                if ((counter % 100) == 0) {
                    printf("Execution %u...\r\n", counter);
                }
            }
        }

        printf("Reset EdgeTPU...\r\n");
        coral::micro::EdgeTpuTask::GetSingleton()->SetPower(false);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskSuspend(NULL);
}
