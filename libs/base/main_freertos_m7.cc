#include "libs/base/console_m7.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/random.h"
#include "libs/base/tasks_m7.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

lpi2c_rtos_handle_t i2c5_handle;

extern "C" void app_main(void *param);
extern "C" void BOARD_InitHardware();

extern "C" int main(int argc, char **argv) __attribute__((weak));
extern "C" int main(int argc, char **argv) {
    BOARD_InitHardware();
    valiant::Random::GetSingleton()->Init();
    valiant::ConsoleM7::GetSingleton()->Init();
    valiant::IPCInit();
    // Make sure this happens before EEM or WICED are initialized.
    tcpip_init(NULL, NULL);
    valiant::UsbDeviceTask::GetSingleton()->Init();
    valiant::UsbHostTask::GetSingleton()->Init();
    valiant::EdgeTpuDfuTask::GetSingleton()->Init();
    valiant::EdgeTpuTask::GetSingleton()->Init();

#if defined(BOARD_REVISION_P0)
    // Initialize I2C5 state
    NVIC_SetPriority(LPI2C5_IRQn, 3);
    lpi2c_master_config_t config;
    LPI2C_MasterGetDefaultConfig(&config);
    LPI2C_RTOS_Init(&i2c5_handle, (LPI2C_Type*)LPI2C5_BASE, &config, CLOCK_GetFreq(kCLOCK_OscRc48MDiv2));

    valiant::PmicTask::GetSingleton()->Init(&i2c5_handle);
    valiant::CameraTask::GetSingleton()->Init(&i2c5_handle);
    valiant::AudioTask::GetSingleton()->Init();
#endif

    xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE * 10, NULL, APP_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    return 0;
}
