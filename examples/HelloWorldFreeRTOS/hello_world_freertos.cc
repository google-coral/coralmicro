#include <cstdio>

#include "libs/base/console_m7.h"
#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

[[noreturn]] void read_task(void* param) {
    char ch;
    do {
        int bytes = coral::micro::ConsoleM7::GetSingleton()->Read(&ch, 1);
        if (bytes == 1) {
            coral::micro::ConsoleM7::GetSingleton()->Write(&ch, 1);
        }
        taskYIELD();
    } while (true);
}

extern "C" [[noreturn]] void app_main(void* param) {
    printf("Hello world FreeRTOS.\r\n");

    coral::micro::PmicTask::GetSingleton()->SetRailState(
        coral::micro::PmicRail::kCam2V8, true);
    coral::micro::PmicTask::GetSingleton()->SetRailState(
        coral::micro::PmicRail::kCam1V8, true);
    coral::micro::PmicTask::GetSingleton()->SetRailState(
        coral::micro::PmicRail::kMic1V8, true);
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);

    xTaskCreate(read_task, "read_task", configMINIMAL_STACK_SIZE, nullptr,
                APP_TASK_PRIORITY, nullptr);

    bool up = true;
    unsigned int brightness = 50;
    while (true) {
        coral::micro::led::Set(coral::micro::led::LED::kPower, brightness > 50);
        coral::micro::led::Set(coral::micro::led::LED::kUser, brightness > 50);
        coral::micro::led::Set(coral::micro::led::LED::kTpu, true, brightness);

        if (up) {
            ++brightness;
        } else {
            --brightness;
        }
        if (brightness == 100) {
            up = false;
        }
        if (brightness == 0) {
            up = true;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskSuspend(nullptr);
}
