#include "libs/base/gpio.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M7.\r\n");
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    bool on = true;
    while (true) {
        on = !on;
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kPowerLED, on);
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kUserLED, on);
        coral::micro::gpio::SetGpio(coral::micro::gpio::Gpio::kTpuLED, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
