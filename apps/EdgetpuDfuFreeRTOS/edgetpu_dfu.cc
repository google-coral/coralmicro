#include "libs/base/tasks_m7.h"
#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <cstdio>

// Measured stack usage: 242 words
void USB_HostTask(void *param) {
    while (true) {
        valiant::UsbHostTask::GetSingleton()->UsbHostTaskFn();
        taskYIELD();
    }
}

// Measured stack usage: 146 words
void USB_DfuTask(void *param) {
    while (true) {
        valiant::EdgeTpuDfuTask::GetSingleton()->EdgeTpuDfuTaskFn();
        taskYIELD();
    }
}

extern "C" void app_main(void *param) {
    int ret;

    ret = xTaskCreate(USB_HostTask, "USB_HostTask", configMINIMAL_STACK_SIZE * 3, NULL, APP_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        printf("Failed to start USB_HostTask\r\n");
        return;
    }

    ret = xTaskCreate(USB_DfuTask, "USB_DfuTask", configMINIMAL_STACK_SIZE * 3, NULL, APP_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        printf("Failed to start USB_DfuTask\r\n");
        return;
    }

    while (true) {
        taskYIELD();
    }
}
