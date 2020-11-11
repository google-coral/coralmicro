#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"

extern "C" void main_task(osa_task_param_t arg) {
    while (true) {
        valiant::UsbHostTask::GetSingleton()->UsbHostTaskFn();
        valiant::EdgeTpuDfuTask::GetSingleton()->EdgeTpuDfuTaskFn();
    }
}
