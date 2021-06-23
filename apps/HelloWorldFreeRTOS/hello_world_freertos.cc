#include "libs/base/gpio.h"
#include "libs/base/console_m7.h"
#include "libs/base/tasks.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

void read_task(void* param) {
    char ch;
    do {
        int bytes = valiant::ConsoleM7::GetSingleton()->Read(&ch, 1);
        if (bytes == 1) {
            valiant::ConsoleM7::GetSingleton()->Write(&ch, 1);
        }
        taskYIELD();
    } while (true);
}

extern "C" void app_main(void *param) {
    printf("Hello world FreeRTOS.\r\n");

    xTaskCreate(read_task, "read_task", configMINIMAL_STACK_SIZE, nullptr, APP_TASK_PRIORITY, nullptr);
    bool on = true;
    while (true) {
        on = !on;
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kPowerLED, on);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
