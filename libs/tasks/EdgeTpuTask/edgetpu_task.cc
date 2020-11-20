#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/usb_host_edgetpu/usb_host_edgetpu.h"

#include <cstdio>
#include <functional>

using namespace std::placeholders;

namespace valiant {

void EdgeTpuTask::SetNextState(enum edgetpu_state next_state) {
    OSA_MsgQPut(message_queue_, &next_state);
}

usb_status_t EdgeTpuTask::USBHostEvent(
            usb_host_handle host_handle,
            usb_device_handle device_handle,
            usb_host_configuration_handle config_handle,
            uint32_t event_code) {
    uint8_t id;
    usb_host_configuration_t *configuration_ptr;
    usb_host_interface_t *interface_ptr;
    switch (event_code & 0xFFFF) {
        case kUSB_HostEventAttach:
            configuration_ptr = (usb_host_configuration_t*)config_handle;
            for (unsigned int i = 0; i < configuration_ptr->interfaceCount; ++i) {
                interface_ptr = &configuration_ptr->interfaceList[i];
                id = interface_ptr->interfaceDesc->bInterfaceClass;

                if (id != USB_HOST_EDGETPU_CLASS_CODE) {
                    continue;
                }

                id = interface_ptr->interfaceDesc->bInterfaceSubClass;
                if (id == USB_HOST_EDGETPU_SUBCLASS_CODE) {
                    SetDeviceHandle(device_handle);
                    SetInterfaceHandle(interface_ptr);
                    break;
                }
            }
            return (this->device_handle() != nullptr) ? kStatus_USB_Success : kStatus_USB_NotSupported;
        case kUSB_HostEventEnumerationDone:
            SetNextState(EDGETPU_STATE_ATTACHED);
            return kStatus_USB_Success;
        case kUSB_HostEventDetach:
            SetNextState(EDGETPU_STATE_UNATTACHED);
            return kStatus_USB_Success;
        default:
            return kStatus_USB_Success;
    }
}

void EdgeTpuTask::SetInterfaceCallback(void *param, uint8_t *data, uint32_t data_length, usb_status_t status) {
    EdgeTpuTask *task = (EdgeTpuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in EdgeTpuSetInterface\r\n");
        task->SetNextState(EDGETPU_STATE_ERROR);
        return;
    }
    task->SetNextState(EDGETPU_STATE_GET_STATUS);
}

void EdgeTpuTask::GetStatusCallback(void *param, uint8_t *data, uint32_t data_length, usb_status_t status) {
    EdgeTpuTask *task = (EdgeTpuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in EdgeTpuGetStatus\r\n");
        task->SetNextState(EDGETPU_STATE_ERROR);
        return;
    }
    task->SetNextState(EDGETPU_STATE_CONNECTED);
}

EdgeTpuTask::EdgeTpuTask() {
    OSA_MsgQCreate((osa_msgq_handle_t)message_queue_, 1U, sizeof(uint32_t));
    valiant::UsbHostTask::GetSingleton()->RegisterUSBHostEventCallback(
            kEdgeTpuVid, kEdgeTpuPid, std::bind(&EdgeTpuTask::USBHostEvent, this, _1, _2, _3, _4));
}

void EdgeTpuTask::EdgeTpuTaskFn() {
    usb_status_t ret;
    enum edgetpu_state next_state;
    if (OSA_MsgQGet(message_queue_, &next_state, osaWaitForever_c) != KOSA_StatusSuccess) {
        return;
    }

    switch (next_state) {
        case EDGETPU_STATE_UNATTACHED:
            break;
        case EDGETPU_STATE_ATTACHED:
            ret = USB_HostEdgeTpuInit(device_handle(), &class_handle_);
            if (ret != kStatus_USB_Success) {
                SetNextState(EDGETPU_STATE_ERROR);
            } else {
                SetNextState(EDGETPU_STATE_SET_INTERFACE);
            }
            break;
        case EDGETPU_STATE_SET_INTERFACE:
            ret = USB_HostEdgeTpuSetInterface(
                    class_handle(), interface_handle(),
                    0, SetInterfaceCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(EDGETPU_STATE_ERROR);
            }
            // Thunderchild notifies EdgeTpuManager that it's connected. Should this be in callback?
            EdgeTpuManager::GetSingleton()->NotifyConnected((usb_host_edgetpu_instance_t*)class_handle());
            break;
        case EDGETPU_STATE_GET_STATUS:
            ret = USB_HostEdgeTpuGetStatus(class_handle(), &status_, GetStatusCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(EDGETPU_STATE_ERROR);
            }
            break;
        // case EDGETPU_STATE_CONNECTED:
        //     EdgeTpuManager::GetSingleton()->NotifyConnected((usb_host_edgetpu_instance_t*)class_handle());
        //     break;
        case EDGETPU_STATE_ERROR:
            printf("EdgeTPU error\r\n");
            while (true) {
                taskYIELD();
            }
            break;
        default:
            printf("Unhandled EdgeTPU state: %d\r\n", next_state);
            while (true) {
                taskYIELD();
            }
            break;
    }
}

}  // namespace valiant
