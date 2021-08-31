#include "libs/base/main_freertos_m7.h"

#include "libs/base/console_m7.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/ipc.h"
#include "libs/base/random.h"
#include "libs/base/tasks.h"
#include "libs/base/timer.h"
#include "libs/CdcEem/cdc_eem.h"
#include "libs/nxp/rt1176-sdk/board_hardware.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "libs/tasks/PowerMonitorTask/power_monitor_task.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"
#include <functional>
using namespace std::placeholders;

lpi2c_rtos_handle_t i2c5_handle;
static valiant::CdcEem cdc_eem;

void InitializeCDCEEM() {
    cdc_eem.Init(
            valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_interface_value());
    valiant::UsbDeviceTask::GetSingleton()->AddDevice(cdc_eem.config_data(),
            std::bind(&valiant::CdcEem::SetClassHandle, &cdc_eem, _1),
            std::bind(&valiant::CdcEem::HandleEvent, &cdc_eem, _1, _2),
            cdc_eem.descriptor_data(), cdc_eem.descriptor_data_size());
}

extern "C" void app_main(void *param);
extern "C" int main(int argc, char **argv) __attribute__((weak));

extern "C" int main(int argc, char **argv) {
    return real_main(argc, argv, true, true);
}

extern "C" int real_main(int argc, char **argv, bool init_console_tx, bool init_console_rx) {
    BOARD_InitHardware(true);
    valiant::timer::Init();
    valiant::gpio::Init();
    valiant::IPC::GetSingleton()->Init();
    valiant::Random::GetSingleton()->Init();
    valiant::ConsoleM7::GetSingleton()->Init(init_console_tx, init_console_rx);
    assert(valiant::filesystem::Init());
    // Make sure this happens before EEM or WICED are initialized.
    tcpip_init(NULL, NULL);
    InitializeCDCEEM();
    valiant::UsbDeviceTask::GetSingleton()->Init();
    valiant::UsbHostTask::GetSingleton()->Init();
    valiant::EdgeTpuDfuTask::GetSingleton()->Init();
    valiant::EdgeTpuTask::GetSingleton()->Init();

#if defined(BOARD_REVISION_P0) || defined(BOARD_REVISION_P1)
    // Initialize I2C5 state
    NVIC_SetPriority(LPI2C5_IRQn, 3);
    lpi2c_master_config_t config;
    LPI2C_MasterGetDefaultConfig(&config);
    LPI2C_RTOS_Init(&i2c5_handle, (LPI2C_Type*)LPI2C5_BASE, &config, CLOCK_GetFreq(kCLOCK_OscRc48MDiv2));

    valiant::PmicTask::GetSingleton()->Init(&i2c5_handle);
    valiant::PowerMonitorTask::GetSingleton()->Init(&i2c5_handle);
    valiant::CameraTask::GetSingleton()->Init(&i2c5_handle);
    valiant::AudioTask::GetSingleton()->Init();
#endif

    xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE * 10, NULL, APP_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    return 0;
}
