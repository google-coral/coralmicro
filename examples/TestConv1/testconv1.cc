#include "libs/testconv1/testconv1.h"

#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/projdefs.h"

extern "C" [[noreturn]] void app_main(void* param) {
    if (!coral::micro::testconv1::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }

    size_t counter = 0;
    while (true) {
        auto tpu_context =
            coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();
        coral::micro::led::Set(coral::micro::led::LED::kTpu, true);

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
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskSuspend(nullptr);
}
