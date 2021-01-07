#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"

extern "C" void app_main(void *param) {
    while (true) {
        valiant::UsbHostTask::GetSingleton()->UsbHostTaskFn();
        valiant::EdgeTpuDfuTask::GetSingleton()->EdgeTpuDfuTaskFn();
    }
}
