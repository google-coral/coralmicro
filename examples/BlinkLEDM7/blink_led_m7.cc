#include "libs/base/gpio.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M7.\r\n");
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    bool on = true;
    while (true) {
        on = !on;
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kPowerLED, on);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, on);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kTpuLED, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
