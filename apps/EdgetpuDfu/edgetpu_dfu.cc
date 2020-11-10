#include "libs/tasks/EdgeTpuDfuTask/edgetpu_dfu_task.h"
#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/usb_host_config.h"
#include "third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/MIMXRT1176_cm7.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.h"

#include <cstdio>

constexpr int kUSBControllerId = kUSB_ControllerEhci0;
static usb_host_handle gUSBHostHandle;
static int device_class = 0;

extern "C" void USB_OTG1_IRQHandler(void) {
    USB_HostEhciIsrFunction(gUSBHostHandle);
}

static usb_status_t USB_HostEvent(usb_device_handle device_handle,
                                  usb_host_configuration_handle config_handle,
                                  uint32_t event_code) {
    uint32_t vid, pid;
    USB_HostHelperGetPeripheralInformation(device_handle, kUSB_HostGetDeviceVID, &vid);
    USB_HostHelperGetPeripheralInformation(device_handle, kUSB_HostGetDevicePID, &pid);
    printf("USB_HostEvent event_code: %lu vid: 0x%lx pid: 0x%lx\r\n",
           event_code, vid, pid);
    usb_status_t ret = kStatus_USB_Error;
    switch (event_code) {
        case kUSB_HostEventAttach:
            ret = valiant::EdgeTpuDfuTask::GetSingleton()->USB_DFUHostEvent(gUSBHostHandle, device_handle, config_handle, event_code);
            if (ret == kStatus_USB_Success) {
                device_class = 1;
            }
            return ret;
        case kUSB_HostEventEnumerationDone:
            if (device_class == 1) {
                ret = valiant::EdgeTpuDfuTask::GetSingleton()->USB_DFUHostEvent(gUSBHostHandle, device_handle, config_handle, event_code);
            }
            return ret;
        case kUSB_HostEventDetach:
            if (device_class == 1) {
                ret = valiant::EdgeTpuDfuTask::GetSingleton()->USB_DFUHostEvent(gUSBHostHandle, device_handle, config_handle, event_code);
            }
            device_class = 0;
            return ret;
        default:
            break;
    }
    return kStatus_USB_Success;
}

static void USB_HostApplicationInit() {
    usb_status_t status;
    uint32_t usbClockFreq = 24000000;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
    CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    USB_EhciPhyInit(kUSBControllerId, BOARD_XTAL0_CLK_HZ, &phyConfig);

    status = USB_HostInit(kUSBControllerId, &gUSBHostHandle, USB_HostEvent);
    if (status != kStatus_USB_Success) {
        printf("USB_HostInit failed\r\n");
        return;
    }

    IRQn_Type irqNumber = USB_OTG1_IRQn;
    NVIC_SetPriority(irqNumber, 6);
    EnableIRQ(irqNumber);

    uint32_t usb_version;
    USB_HostGetVersion(&usb_version);
    printf("USB host stack version: %lu.%lu.%lu\r\n", (usb_version >> 16) & 0xFF, (usb_version >> 8) & 0xFF, usb_version & 0xFF);
}

extern "C" void main_task(osa_task_param_t arg) {
    USB_HostApplicationInit();

    while (true) {
        USB_HostEhciTaskFunction(gUSBHostHandle);
        valiant::EdgeTpuDfuTask::GetSingleton()->USB_DFUTask();
    }
}
