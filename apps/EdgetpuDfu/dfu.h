#ifndef _APPS_EDGETPUDFU_DFU_H_
#define _APPS_EDGETPUDFU_DFU_H_

#include "libs/nxp/rt1176-sdk/usb_host_config.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"

constexpr int kDfuVid = 0x1A6E;
constexpr int kDfuPid = 0x089A;
extern unsigned char apex_latest_single_ep_bin[];
extern unsigned int apex_latest_single_ep_bin_len;

void USB_DFUTask();

usb_status_t USB_DFUHostEvent(usb_host_handle host_handle, usb_device_handle device_handle,
                              usb_host_configuration_handle config_handle,
                              uint32_t event_code);

#endif  // _APPS_LSUSB_DFU_H_
