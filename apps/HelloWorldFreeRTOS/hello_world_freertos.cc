#include "apps/HelloWorld/board.h"
#include "apps/HelloWorld/peripherals.h"
#include "apps/HelloWorld/pin_mux.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

static void hello_task(void *param) {
    PRINTF("Hello world FreeRTOS.\r\n");
    vTaskSuspend(NULL);
}

int main(int argc, char** argv) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    int ret;
    ret = xTaskCreate(hello_task, "HelloTask", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    if (ret != pdPASS) {
        PRINTF("Failed to start HelloTask\r\n");
    }

    vTaskStartScheduler();
    while (true);
    return 0;
}
