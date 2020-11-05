#include "apps/EdgetpuDfu/dfu.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_devices.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_ehci.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.h"

#include <algorithm>

static usb_host_instance_t* gHostInstance = nullptr;
static usb_device_handle gDFUDeviceHandle;
static usb_host_interface_handle gDFUInterfaceHandle;
static usb_host_class_handle gDFUClassHandle;
static usb_host_dfu_status_t gDFUStatus;
static int gDFUBytesTransferred = 0;
static int gDFUBytesToTransfer = apex_latest_single_ep_bin_len;
static int gDFUCurrentBlockNum = 0;
static uint8_t *gDFUReadBackData = nullptr;

enum dfu_state {
    DFU_STATE_UNATTACHED = 0,
    DFU_STATE_ATTACHED,
    DFU_STATE_SET_INTERFACE,
    DFU_STATE_GET_STATUS,
    DFU_STATE_TRANSFER,
    DFU_STATE_ZERO_LENGTH_TRANSFER,
    DFU_STATE_READ_BACK,
    DFU_STATE_GET_STATUS_READ,
    DFU_STATE_DETACH,
    DFU_STATE_CHECK_STATUS,
    DFU_STATE_COMPLETE,
    DFU_STATE_WAIT,
    DFU_STATE_ERROR,
};
static dfu_state gDFUState = DFU_STATE_UNATTACHED;

usb_status_t USB_DFUHostEvent(usb_host_handle host_handle, usb_device_handle device_handle,
                              usb_host_configuration_handle config_handle,
                              uint32_t event_code) {
    usb_host_configuration_t *configuration_ptr;
    usb_host_interface_t *interface_ptr;
    int id;
    gHostInstance = (usb_host_instance_t*)host_handle;
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
                    gDFUDeviceHandle = device_handle;
                    gDFUInterfaceHandle = interface_ptr;
                    break;
                }
            }
            return (gDFUDeviceHandle != nullptr) ? kStatus_USB_Success : kStatus_USB_NotSupported;
        case kUSB_HostEventEnumerationDone:
            // TODO: check if we're already dfuing, if handles are valid.
            gDFUState = DFU_STATE_ATTACHED;
            return kStatus_USB_Success;
        case kUSB_HostEventDetach:
            gDFUState = DFU_STATE_UNATTACHED;
            PRINTF("Detached DFU\r\n");
            return kStatus_USB_Success;
        default:
            return kStatus_USB_Success;
    }
}

static void USB_DFUSetInterfaceCallback(void *param,
                                        uint8_t *data,
                                        uint32_t data_length,
                                        usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUSetInterface\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }
    gDFUState = DFU_STATE_GET_STATUS;
}

static void USB_DFUGetStatusCallback(void *param,
                                     uint8_t *data,
                                     uint32_t data_length,
                                     usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUGetStatus\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }

    if (gDFUBytesTransferred < gDFUBytesToTransfer) {
        gDFUState = DFU_STATE_TRANSFER;
    } else {
        gDFUState = DFU_STATE_ZERO_LENGTH_TRANSFER;
    }
}

static void USB_DFUTransferCallback(void *param,
                                    uint8_t *data,
                                    uint32_t data_length,
                                    usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUTransfer\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }

    gDFUCurrentBlockNum++;
    gDFUBytesTransferred += data_length;
    if (gDFUCurrentBlockNum % 10 == 0 || gDFUBytesTransferred == gDFUBytesToTransfer) {
        PRINTF("Transferred %d bytes\r\n", gDFUBytesTransferred);
    }
    gDFUState = DFU_STATE_GET_STATUS;
}

static void USB_DFUZeroLengthTransferCallback(void *param,
                                              uint8_t *data,
                                              uint32_t data_length,
                                              usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUZeroLengthTransfer\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }

    gDFUCurrentBlockNum = 0;
    gDFUBytesTransferred = 0;
    gDFUState = DFU_STATE_READ_BACK;
}

static void USB_DFUReadBackCallback(void *param,
                                    uint8_t *data,
                                    uint32_t data_length,
                                    usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUReadBack\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }

    gDFUCurrentBlockNum++;
    gDFUBytesTransferred += data_length;
    if (gDFUCurrentBlockNum % 10 == 0 || gDFUBytesTransferred == gDFUBytesToTransfer) {
        PRINTF("Read back %d bytes\r\n", gDFUBytesTransferred);
    }
    gDFUState = DFU_STATE_GET_STATUS_READ;
}

static void USB_DFUGetStatusReadCallback(void *param,
                                         uint8_t *data,
                                         uint32_t data_length,
                                         usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUGetStatusRead\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }

    if (gDFUBytesTransferred < gDFUBytesToTransfer) {
        gDFUState = DFU_STATE_READ_BACK;
    } else {
        if (memcmp(apex_latest_single_ep_bin, gDFUReadBackData, apex_latest_single_ep_bin_len) != 0) {
            PRINTF("Read back firmware does not match!\r\n");
            gDFUState = DFU_STATE_ERROR;
        } else {
            gDFUState = DFU_STATE_DETACH;
        }
        OSA_MemoryFree(gDFUReadBackData);
        gDFUReadBackData = nullptr;
        gDFUCurrentBlockNum = 0;
        gDFUBytesTransferred = 0;
    }
}

static void USB_DFUDetachCallback(void *param,
                                  uint8_t *data,
                                  uint32_t data_length,
                                  usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUDetach\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }
    gDFUState = DFU_STATE_CHECK_STATUS;
}

static void USB_DFUCheckStatusCallback(void *param,
                                       uint8_t *data,
                                       uint32_t data_length,
                                       usb_status_t status) {
    if (status != kStatus_USB_Success) {
        PRINTF("Error in DFUCheckStatus\r\n");
        gDFUState = DFU_STATE_ERROR;
        return;
    }
    gDFUState = DFU_STATE_COMPLETE;
}


void USB_DFUTask() {
    usb_status_t ret;
    uint32_t transfer_length;
    switch (gDFUState) {
        case DFU_STATE_UNATTACHED:
            break;
        case DFU_STATE_ATTACHED:
            ret = USB_HostDfuInit(gDFUDeviceHandle, &gDFUClassHandle);
            if (ret == kStatus_USB_Success) {
                gDFUState = DFU_STATE_SET_INTERFACE;
            } else {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_SET_INTERFACE:
            gDFUState = DFU_STATE_WAIT;
            ret = USB_HostDfuSetInterface(gDFUClassHandle, 
                                          gDFUInterfaceHandle,
                                          0,
                                          USB_DFUSetInterfaceCallback,
                                          nullptr);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_GET_STATUS:
            gDFUState = DFU_STATE_WAIT;
            ret = USB_HostDfuGetStatus(gDFUClassHandle,
                                       (uint8_t*)&gDFUStatus,
                                       USB_DFUGetStatusCallback,
                                       nullptr);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_TRANSFER:
            gDFUState = DFU_STATE_WAIT;
            transfer_length = std::min(256U /* get from descriptor */,
                                       apex_latest_single_ep_bin_len - gDFUBytesTransferred);
            ret = USB_HostDfuDnload(gDFUClassHandle,
                                    gDFUCurrentBlockNum,
                                    apex_latest_single_ep_bin + gDFUBytesTransferred,
                                    transfer_length,
                                    USB_DFUTransferCallback,
                                    0);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_ZERO_LENGTH_TRANSFER:
            gDFUState = DFU_STATE_WAIT;
            ret = USB_HostDfuDnload(gDFUClassHandle, gDFUCurrentBlockNum, nullptr, 0,
                                    USB_DFUZeroLengthTransferCallback, nullptr);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_READ_BACK:
            gDFUState = DFU_STATE_WAIT;
            if (!gDFUReadBackData) {
                gDFUReadBackData = (uint8_t*)OSA_MemoryAllocate(apex_latest_single_ep_bin_len);
            }
            transfer_length = std::min(256U /* get from descriptor */,
                                       apex_latest_single_ep_bin_len - gDFUBytesTransferred);
            ret = USB_HostDfuUpload(gDFUClassHandle, gDFUCurrentBlockNum, gDFUReadBackData + gDFUBytesTransferred,
                                    transfer_length, USB_DFUReadBackCallback, nullptr);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_GET_STATUS_READ:
            gDFUState = DFU_STATE_WAIT;
            ret = USB_HostDfuGetStatus(gDFUClassHandle, (uint8_t*)&gDFUStatus, USB_DFUGetStatusReadCallback, nullptr);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_DETACH:
            gDFUState = DFU_STATE_WAIT;
            ret = USB_HostDfuDetach(gDFUClassHandle, 1000 /* ms */, USB_DFUDetachCallback, nullptr);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_CHECK_STATUS:
            gDFUState = DFU_STATE_WAIT;
            ret = USB_HostDfuGetStatus(gDFUClassHandle, (uint8_t*)&gDFUStatus, USB_DFUCheckStatusCallback, nullptr);
            if (ret != kStatus_USB_Success) {
                gDFUState = DFU_STATE_ERROR;
            }
            break;
        case DFU_STATE_COMPLETE:
            USB_HostDfuDeinit(gDFUDeviceHandle, gDFUClassHandle);
            gDFUClassHandle = nullptr;
            USB_HostEhciResetBus((usb_host_ehci_instance_t*)gHostInstance->controllerHandle);
            USB_HostTriggerReEnumeration(gDFUDeviceHandle);
            gDFUState = DFU_STATE_UNATTACHED;
            break;
        case DFU_STATE_WAIT:
            break;
        case DFU_STATE_ERROR:
            PRINTF("DFU error\r\n");
            while (true);
            break;
        default:
            PRINTF("Unhandled DFU state %d\r\n", gDFUState);
            while (true);
            break;
    }
}
