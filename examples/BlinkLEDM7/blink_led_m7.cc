#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void *param) {
    printf("Blinking LED from M7.\r\n");
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    bool on = true;
    while (true) {
        on = !on;
        coral::micro::led::Set(coral::micro::led::LED::kPower, on);
        coral::micro::led::Set(coral::micro::led::LED::kUser, on);
        coral::micro::led::Set(coral::micro::led::LED::kTpu, on);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
