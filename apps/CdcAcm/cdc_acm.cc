#include "libs/CdcAcm/cdc_acm.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstdio>

using namespace std::placeholders;

TaskHandle_t echo_task_handle;
static void ReceiveHandler(const uint8_t *buffer, const uint32_t length) {
    xTaskNotifyGive(echo_task_handle);
}

static void echo_task(void *param) {
    const char *fixed_str = "Hello from CdcAcm\r\n";
    valiant::CdcAcm* cdc_acm = (valiant::CdcAcm*)param;
    while (true) {
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ulNotificationValue == 1) {
            cdc_acm->Transmit((const uint8_t*)fixed_str, strlen(fixed_str));
        }
    }
}

static void usb_device_task(void *param) {
    int ret = false;
    valiant::CdcAcm cdc_acm(0,1,1,2,1,
            ReceiveHandler); // TODO(atv): lol
    ret = valiant::UsbDeviceTask::GetSingleton()->Init(cdc_acm.config_data(),
            std::bind(&valiant::CdcAcm::SetClassHandle, &cdc_acm, _1),
            std::bind(&valiant::CdcAcm::HandleEvent, &cdc_acm, _1, _2));
    if (!ret) {
        printf("Failed to bring up USB CDC\r\n");
        return;
    }

    ret = xTaskCreate(echo_task, "EchoTask", configMINIMAL_STACK_SIZE * 10, &cdc_acm, configMAX_PRIORITIES - 1, &echo_task_handle);
    if (ret != pdPASS) {
        printf("Failed to start ConsoleTask\r\n");
        return;
    }

    while (true) {
        valiant::UsbDeviceTask::GetSingleton()->UsbDeviceTaskFn();
        taskYIELD();
    }
}

extern "C" void app_main(void *param) {
    int ret;
    printf("CdcAcm\r\n");
    ret = xTaskCreate(usb_device_task, "UsbDeviceTask", configMINIMAL_STACK_SIZE * 10, NULL, configMAX_PRIORITIES - 1, NULL);
    if (ret != pdPASS) {
        printf("Failed to start UsbDeviceTask\r\n");
        return;
    }
    while (true) {
        taskYIELD();
    }
}
