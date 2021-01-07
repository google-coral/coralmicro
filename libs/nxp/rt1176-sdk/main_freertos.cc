#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

extern "C" void app_main(void *param);
extern "C" void BOARD_InitHardware();

extern "C" int main(int argc, char **argv) __attribute__((weak));
extern "C" int main(int argc, char **argv) {
    BOARD_InitHardware();

    xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    vTaskStartScheduler();

    return 0;
}
