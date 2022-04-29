#include "libs/base/gpio.h"
#include "libs/base/console_m7.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

void read_task(void* param) {
    char ch;
    do {
        int bytes = coral::micro::ConsoleM7::GetSingleton()->Read(&ch, 1);
        if (bytes == 1) {
            coral::micro::ConsoleM7::GetSingleton()->Write(&ch, 1);
        }
        taskYIELD();
    } while (true);
}

extern "C" void app_main(void *param) {
    printf("Hello world FreeRTOS.\r\n");

    coral::micro::PmicTask::GetSingleton()->SetRailState(coral::micro::pmic::Rail::CAM_2V8, true);
    coral::micro::PmicTask::GetSingleton()->SetRailState(coral::micro::pmic::Rail::CAM_1V8, true);
    coral::micro::PmicTask::GetSingleton()->SetRailState(coral::micro::pmic::Rail::MIC_1V8, true);
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);

    xTaskCreate(read_task, "read_task", configMINIMAL_STACK_SIZE, nullptr, APP_TASK_PRIORITY, nullptr);
    bool on = true;
    while (true) {
        on = !on;
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kPowerLED, on);
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kUserLED, on);
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kTpuLED, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
