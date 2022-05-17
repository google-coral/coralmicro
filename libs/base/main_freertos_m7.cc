#include "libs/base/main_freertos_m7.h"

#include <functional>

#include "libs/base/check.h"
#include "libs/base/console_m7.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/reset.h"
#include "libs/base/random.h"
#include "libs/base/tasks.h"
#include "libs/base/tempsense.h"
#include "libs/base/timer.h"
#include "libs/cdc_eem/cdc_eem.h"
#include "libs/nxp/rt1176-sdk/board_hardware.h"
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
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_sema4.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

namespace {
lpi2c_rtos_handle_t i2c5_handle;
coral::micro::CdcEem cdc_eem;

void InitializeCDCEEM() {
    using namespace std::placeholders;
    cdc_eem.Init(
        coral::micro::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
        coral::micro::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
        coral::micro::UsbDeviceTask::GetSingleton()->next_interface_value());
    coral::micro::UsbDeviceTask::GetSingleton()->AddDevice(
        cdc_eem.config_data(),
        std::bind(&coral::micro::CdcEem::SetClassHandle, &cdc_eem, _1),
        std::bind(&coral::micro::CdcEem::HandleEvent, &cdc_eem, _1, _2),
        cdc_eem.descriptor_data(), cdc_eem.descriptor_data_size());
}
}  // namespace

extern "C" lpi2c_rtos_handle_t* I2C5Handle() { return &i2c5_handle; }

extern "C" void app_main(void* param);
extern "C" int main(int argc, char** argv) __attribute__((weak));

extern "C" int main(int argc, char** argv) {
    return real_main(argc, argv, true, true);
}

extern "C" int real_main(int argc, char** argv, bool init_console_tx,
                         bool init_console_rx) {
    BOARD_InitHardware(true);
    SEMA4_Init(SEMA4);
    coral::micro::StoreResetReason();
    coral::micro::timer::Init();
    coral::micro::gpio::Init();
    coral::micro::IPCM7::GetSingleton()->Init();
    coral::micro::Random::GetSingleton()->Init();
    coral::micro::ConsoleM7::GetSingleton()->Init(init_console_tx,
                                                  init_console_rx);
    CHECK(coral::micro::filesystem::Init());
    // Make sure this happens before EEM or WICED are initialized.
    tcpip_init(nullptr, nullptr);
    InitializeCDCEEM();
    coral::micro::UsbDeviceTask::GetSingleton()->Init();
    coral::micro::UsbHostTask::GetSingleton()->Init();
    coral::micro::EdgeTpuDfuTask::GetSingleton()->Init();
    coral::micro::EdgeTpuTask::GetSingleton()->Init();
    coral::micro::tempsense::Init();

#if defined(BOARD_REVISION_P0) || defined(BOARD_REVISION_P1)
    // Initialize I2C5 state
    NVIC_SetPriority(LPI2C5_IRQn, 3);
    lpi2c_master_config_t config;
    LPI2C_MasterGetDefaultConfig(&config);
    LPI2C_RTOS_Init(&i2c5_handle, (LPI2C_Type*)LPI2C5_BASE, &config,
                    CLOCK_GetFreq(kCLOCK_OscRc48MDiv2));

    coral::micro::PmicTask::GetSingleton()->Init(&i2c5_handle);
    coral::micro::CameraTask::GetSingleton()->Init(&i2c5_handle);
#endif

    CHECK(xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE * 30,
                      nullptr, APP_TASK_PRIORITY, nullptr) == pdPASS);

    vTaskStartScheduler();
    return 0;
}
