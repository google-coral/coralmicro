#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_devices.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_ehci.h"

#include <algorithm>
#include <cstdio>
#include <functional>

using namespace std::placeholders;

namespace valiant {

void EdgeTpuDfuTask::SetNextState(enum dfu_state next_state) {
    OSA_MsgQPut(message_queue_, &next_state);
}

usb_status_t EdgeTpuDfuTask::USB_DFUHostEvent(usb_host_handle host_handle,
                                              usb_device_handle device_handle,
                                              usb_host_configuration_handle config_handle,
                                              uint32_t event_code) {
    usb_host_configuration_t *configuration_ptr;
    usb_host_interface_t *interface_ptr;
    int id;
    SetHostInstance((usb_host_instance_t*)host_handle);
    switch (event_code) {
        case kUSB_HostEventAttach:
            configuration_ptr = (usb_host_configuration_t*)config_handle;
            for (int i = 0; i < configuration_ptr->interfaceCount; ++i) {
                interface_ptr = &configuration_ptr->interfaceList[i];
                id = interface_ptr->interfaceDesc->bInterfaceClass;
                if (id != USB_HOST_DFU_CLASS_CODE) {
                    continue;
                }

                id = interface_ptr->interfaceDesc->bInterfaceSubClass;
                if (id == USB_HOST_DFU_SUBCLASS_CODE) {
                    SetDeviceHandle(device_handle);
                    SetInterfaceHandle(interface_ptr);
                    break;
                }
            }
            return (this->device_handle() != nullptr) ? kStatus_USB_Success : kStatus_USB_NotSupported;
        case kUSB_HostEventEnumerationDone:
            // TODO: check if we're already dfuing, if handles are valid.
            SetNextState(DFU_STATE_ATTACHED);
            return kStatus_USB_Success;
        case kUSB_HostEventDetach:
            SetNextState(DFU_STATE_UNATTACHED);
            printf("Detached DFU\r\n");
            return kStatus_USB_Success;
        default:
            return kStatus_USB_Success;
    }
}

void EdgeTpuDfuTask::SetInterfaceCallback(void *param,
                                          uint8_t *data,
                                          uint32_t data_length,
                                          usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUSetInterface\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }
    task->SetNextState(DFU_STATE_GET_STATUS);
}

void EdgeTpuDfuTask::GetStatusCallback(void *param,
                                       uint8_t *data,
                                       uint32_t data_length,
                                       usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUGetStatus\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }

    if (task->bytes_transferred() < task->bytes_to_transfer()) {
        task->SetNextState(DFU_STATE_TRANSFER);
    } else {
        task->SetNextState(DFU_STATE_ZERO_LENGTH_TRANSFER);
    }
}

void EdgeTpuDfuTask::TransferCallback(void *param,
                                      uint8_t *data,
                                      uint32_t data_length,
                                      usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUTransfer\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }

    task->SetCurrentBlockNumber(task->current_block_number() + 1);
    task->SetBytesTransferred(task->bytes_transferred() + data_length);
    if (task->current_block_number() % 10 == 0 || task->bytes_transferred() == task->bytes_to_transfer()) {
        printf("Transferred %d bytes\r\n", task->bytes_transferred());
    }
    task->SetNextState(DFU_STATE_GET_STATUS);
}

void EdgeTpuDfuTask::ZeroLengthTransferCallback(void *param,
                                                uint8_t *data,
                                                uint32_t data_length,
                                                usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUZeroLengthTransfer\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }

    task->SetCurrentBlockNumber(0);
    task->SetBytesTransferred(0);
    task->SetNextState(DFU_STATE_READ_BACK);
}

void EdgeTpuDfuTask::ReadBackCallback(void *param,
                                      uint8_t *data,
                                      uint32_t data_length,
                                      usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUReadBack\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }

    task->SetCurrentBlockNumber(task->current_block_number() + 1);
    task->SetBytesTransferred(task->bytes_transferred() + data_length);
    if (task->current_block_number() % 10 == 0 || task->bytes_transferred() == task->bytes_to_transfer()) {
        printf("Read back %d bytes\r\n", task->bytes_transferred());
    }
    task->SetNextState(DFU_STATE_GET_STATUS_READ);
}

void EdgeTpuDfuTask::GetStatusReadCallback(void *param,
                                           uint8_t *data,
                                           uint32_t data_length,
                                           usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUGetStatusRead\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }

    if (task->bytes_transferred() < task->bytes_to_transfer()) {
        task->SetNextState(DFU_STATE_READ_BACK);
    } else {
        if (memcmp(apex_latest_single_ep_bin, task->read_back_data(), apex_latest_single_ep_bin_len) != 0) {
            printf("Read back firmware does not match!\r\n");
            task->SetNextState(DFU_STATE_ERROR);
        } else {
            task->SetNextState(DFU_STATE_DETACH);
        }
        OSA_MemoryFree(task->read_back_data());
        task->SetReadBackData(nullptr);
        task->SetCurrentBlockNumber(0);
        task->SetBytesTransferred(0);
    }
}

void EdgeTpuDfuTask::DetachCallback(void *param,
                                    uint8_t *data,
                                    uint32_t data_length,
                                    usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUDetach\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }
    task->SetNextState(DFU_STATE_CHECK_STATUS);
}

void EdgeTpuDfuTask::CheckStatusCallback(void *param,
                                         uint8_t *data,
                                         uint32_t data_length,
                                         usb_status_t status) {
    EdgeTpuDfuTask *task = (EdgeTpuDfuTask*)param;
    if (status != kStatus_USB_Success) {
        printf("Error in DFUCheckStatus\r\n");
        task->SetNextState(DFU_STATE_ERROR);
        return;
    }
    task->SetNextState(DFU_STATE_COMPLETE);
}

EdgeTpuDfuTask::EdgeTpuDfuTask() {
    OSA_MsgQCreate((osa_msgq_handle_t)message_queue_, 1U, sizeof(uint32_t));
    valiant::UsbHostTask::GetSingleton()->RegisterUSBHostEventCallback(kDfuVid, kDfuPid,
            std::bind(&EdgeTpuDfuTask::USB_DFUHostEvent, this, _1, _2, _3, _4));
}

void EdgeTpuDfuTask::EdgeTpuDfuTaskFn() {
    usb_status_t ret;
    uint32_t transfer_length;
    enum dfu_state next_state;
    if (OSA_MsgQGet(message_queue_, &next_state, osaWaitForever_c) != KOSA_StatusSuccess) {
        return;
    }
    switch (next_state) {
        case DFU_STATE_UNATTACHED:
            break;
        case DFU_STATE_ATTACHED:
            ret = USB_HostDfuInit(device_handle(), &class_handle_);
            if (ret == kStatus_USB_Success) {
                SetNextState(DFU_STATE_SET_INTERFACE);
            } else {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_SET_INTERFACE:
            ret = USB_HostDfuSetInterface(class_handle(), 
                                          interface_handle(),
                                          0,
                                          EdgeTpuDfuTask::SetInterfaceCallback,
                                          this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_GET_STATUS:
            ret = USB_HostDfuGetStatus(class_handle(),
                                       (uint8_t*)&status_,
                                       EdgeTpuDfuTask::GetStatusCallback,
                                       this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_TRANSFER:
            transfer_length = std::min(256U /* get from descriptor */,
                                       apex_latest_single_ep_bin_len - bytes_transferred());
            ret = USB_HostDfuDnload(class_handle(),
                                    current_block_number(),
                                    apex_latest_single_ep_bin + bytes_transferred(),
                                    transfer_length,
                                    EdgeTpuDfuTask::TransferCallback,
                                    this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_ZERO_LENGTH_TRANSFER:
            ret = USB_HostDfuDnload(class_handle(), current_block_number(), nullptr, 0,
                                    EdgeTpuDfuTask::ZeroLengthTransferCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_READ_BACK:
            if (!read_back_data()) {
                SetReadBackData((uint8_t*)OSA_MemoryAllocate(apex_latest_single_ep_bin_len));
            }
            transfer_length = std::min(256U /* get from descriptor */,
                                       apex_latest_single_ep_bin_len - bytes_transferred());
            ret = USB_HostDfuUpload(class_handle(), current_block_number(), read_back_data() + bytes_transferred(),
                                    transfer_length, EdgeTpuDfuTask::ReadBackCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_GET_STATUS_READ:
            ret = USB_HostDfuGetStatus(class_handle(), (uint8_t*)&status_, EdgeTpuDfuTask::GetStatusReadCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_DETACH:
            ret = USB_HostDfuDetach(class_handle(), 1000 /* ms */, EdgeTpuDfuTask::DetachCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_CHECK_STATUS:
            ret = USB_HostDfuGetStatus(class_handle(), (uint8_t*)&status_, EdgeTpuDfuTask::CheckStatusCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(DFU_STATE_ERROR);
            }
            break;
        case DFU_STATE_COMPLETE:
            USB_HostDfuDeinit(device_handle(), class_handle());
            SetClassHandle(nullptr);
            USB_HostEhciResetBus((usb_host_ehci_instance_t*)host_instance()->controllerHandle);
            USB_HostTriggerReEnumeration(device_handle());
            SetNextState(DFU_STATE_UNATTACHED);
            break;
        case DFU_STATE_ERROR:
            printf("DFU error\r\n");
            while (true) {}
            break;
        default:
            printf("Unhandled DFU state %d\r\n", next_state);
            while (true) {}
            break;
    }
}

}  // namespace valiant
