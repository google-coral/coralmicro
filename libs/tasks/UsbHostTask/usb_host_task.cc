#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/clock_config.h"
#include "libs/nxp/rt1176-sdk/usb_host_config.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.h"

#include <cstdio>

constexpr int kUSBControllerId = kUSB_ControllerEhci1;

extern "C" void USB_OTG2_IRQHandler(void) {
    USB_HostEhciIsrFunction(valiant::UsbHostTask::GetSingleton()->host_handle());
}

static uint32_t vidpid_to_key(uint32_t vid, uint32_t pid) {
    return ((vid & 0xFFFF) << 16) | (pid & 0xFFFF);
}

static usb_status_t USB_HostEvent(usb_device_handle device_handle,
                                  usb_host_configuration_handle config_handle,
                                  uint32_t event_code) {
    return valiant::UsbHostTask::GetSingleton()->HostEvent(device_handle, config_handle, event_code);
}

namespace valiant {
usb_status_t UsbHostTask::HostEvent(usb_device_handle device_handle,
                                  usb_host_configuration_handle config_handle,
                                  uint32_t event_code) {
    uint32_t vid, pid, vidpid;

    USB_HostHelperGetPeripheralInformation(device_handle, kUSB_HostGetDeviceVID, &vid);
    USB_HostHelperGetPeripheralInformation(device_handle, kUSB_HostGetDevicePID, &pid);
    printf("USB_HostEvent event_code: %lu status: %lu vid: 0x%lx pid: 0x%lx\r\n",
           event_code & 0xFFFF, event_code >> 16, vid, pid);

    vidpid = vidpid_to_key(vid, pid);
    if (host_event_callbacks_.find(vidpid) == host_event_callbacks_.end()) {
            return kStatus_USB_Error;
    }
    usb_host_event_callback callback_fn = host_event_callbacks_[vidpid];

    return callback_fn(host_handle(), device_handle, config_handle, event_code);
}

void UsbHostTask::RegisterUSBHostEventCallback(uint32_t vid, uint32_t pid, usb_host_event_callback fn) {
    uint32_t vidpid = vidpid_to_key(vid, pid);
    host_event_callbacks_[vidpid] = fn;

    IRQn_Type irqNumber = USB_OTG2_IRQn;
    NVIC_SetPriority(irqNumber, 6);
    NVIC_EnableIRQ(irqNumber);
}

UsbHostTask::UsbHostTask() {
    usb_status_t status;
    uint32_t usbClockFreq = 24000000;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
    CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, usbClockFreq);
    USB_EhciPhyInit(kUSBControllerId, BOARD_XTAL0_CLK_HZ, &phyConfig);

    status = USB_HostInit(kUSBControllerId, &host_handle_, USB_HostEvent);
    if (status != kStatus_USB_Success) {
        printf("USB_HostInit failed\r\n");
        return;
    }

    uint32_t usb_version;
    USB_HostGetVersion(&usb_version);
    printf("USB host stack version: %lu.%lu.%lu\r\n", (usb_version >> 16) & 0xFF, (usb_version >> 8) & 0xFF, usb_version & 0xFF);
}

void UsbHostTask::UsbHostTaskFn() {
    USB_HostEhciTaskFunction(host_handle());
}

}  // namespace valiant
