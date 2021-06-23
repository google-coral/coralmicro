#include "libs/base/console_m4.h"
#include "libs/base/filesystem.h"
#include "libs/base/ipc.h"
#include "libs/base/tasks.h"
#include "libs/nxp/rt1176-sdk/board_hardware.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include <cstdio>
#include <cstring>

lpi2c_rtos_handle_t i2c5_handle;
extern "C" void app_main(void *param);

extern "C" int main(int argc, char **argv) __attribute__((weak));
extern "C" int main(int argc, char **argv) {
    BOARD_InitHardware(true);
    valiant::IPC::GetSingleton()->Init();
    valiant::ConsoleInit();
    valiant::filesystem::Init();

#if defined(BOARD_REVISION_P0)
    // Initialize I2C5 state
    NVIC_SetPriority(LPI2C5_IRQn, 3);
    lpi2c_master_config_t config;
    LPI2C_MasterGetDefaultConfig(&config);
    LPI2C_RTOS_Init(&i2c5_handle, (LPI2C_Type*)LPI2C5_BASE, &config, CLOCK_GetFreq(kCLOCK_OscRc48MDiv2));

    valiant::PmicTask::GetSingleton()->Init(&i2c5_handle);
    valiant::CameraTask::GetSingleton()->Init(&i2c5_handle);
#endif

    static StaticTask_t xTaskBuffer;
    static StackType_t xStack[configMINIMAL_STACK_SIZE * 10];
    xTaskCreateStatic(app_main, "app_main", configMINIMAL_STACK_SIZE * 10, NULL, APP_TASK_PRIORITY, xStack, &xTaskBuffer);

    vTaskStartScheduler();

    return 0;
}
