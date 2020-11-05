#include "apps/EdgetpuDfu/board.h"
#include "apps/EdgetpuDfu/dfu.h"
#include "apps/EdgetpuDfu/peripherals.h"
#include "apps/EdgetpuDfu/pin_mux.h"
#include "libs/nxp/rt1176-sdk/usb_host_config.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/MIMXRT1176_cm7.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.h"

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
    PRINTF("USB_HostEvent event_code: %d vid: 0x%x pid: 0x%x\r\n",
           event_code, vid, pid);
    usb_status_t ret = kStatus_USB_Error;
    switch (event_code) {
        case kUSB_HostEventAttach:
            ret = USB_DFUHostEvent(gUSBHostHandle, device_handle, config_handle, event_code);
            if (ret == kStatus_USB_Success) {
                device_class = 1;
            }
            return ret;
        case kUSB_HostEventEnumerationDone:
            if (device_class == 1) {
                ret = USB_DFUHostEvent(gUSBHostHandle, device_handle, config_handle, event_code);
            }
            return ret;
        case kUSB_HostEventDetach:
            if (device_class == 1) {
                ret = USB_DFUHostEvent(gUSBHostHandle, device_handle, config_handle, event_code);
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
        PRINTF("USB_HostInit failed\r\n");
        return;
    }

    IRQn_Type irqNumber = USB_OTG1_IRQn;
    NVIC_SetPriority(irqNumber, 6);
    EnableIRQ(irqNumber);

    uint32_t usb_version;
    USB_HostGetVersion(&usb_version);
    PRINTF("USB host stack version: %d.%d.%d\r\n", (usb_version >> 16) & 0xFF, (usb_version >> 8) & 0xFF, usb_version & 0xFF);
}

int main(int argc, char** argv) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    USB_HostApplicationInit();

    while (true) {
        USB_HostEhciTaskFunction(gUSBHostHandle);
        USB_DFUTask();
    }
    return 0;
}
