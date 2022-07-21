/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>

#include "libs/base/check.h"
#include "libs/base/tasks.h"
#include "libs/nxp/rt1176-sdk/clock_config.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/modified/nxp/rt1176-sdk/board.h"

/* clang-format off */
#include "third_party/modified/nxp/rt1176-sdk/usb_host_config.h"
#include "libs/usb/usb_host_task.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.h"
/* clang-format on */

extern "C" void USB_OTG2_IRQHandler(void) {
  USB_HostEhciIsrFunction(
      coralmicro::UsbHostTask::GetSingleton()->host_handle());
}

namespace coralmicro {
namespace {
constexpr int kUSBControllerId = kUSB_ControllerEhci1;

uint32_t vidpid_to_key(uint32_t vid, uint32_t pid) {
  return ((vid & 0xFFFF) << 16) | (pid & 0xFFFF);
}

usb_status_t USB_HostEvent(usb_device_handle device_handle,
                           usb_host_configuration_handle config_handle,
                           uint32_t event_code) {
  return UsbHostTask::GetSingleton()->HostEvent(device_handle, config_handle,
                                                event_code);
}
}  // namespace

usb_status_t UsbHostTask::HostEvent(usb_device_handle device_handle,
                                    usb_host_configuration_handle config_handle,
                                    uint32_t event_code) {
  uint32_t vid, pid;
  USB_HostHelperGetPeripheralInformation(device_handle, kUSB_HostGetDeviceVID,
                                         &vid);
  USB_HostHelperGetPeripheralInformation(device_handle, kUSB_HostGetDevicePID,
                                         &pid);
#if 0
    printf("USB_HostEvent event_code: %lu status: %lu vid: 0x%lx pid: 0x%lx\r\n",
           event_code & 0xFFFF, event_code >> 16, vid, pid);
#endif

  const uint32_t vidpid = vidpid_to_key(vid, pid);
  if (host_event_callbacks_.find(vidpid) == host_event_callbacks_.end()) {
    return kStatus_USB_Error;
  }
  auto callback_fn = host_event_callbacks_[vidpid];
  return callback_fn(host_handle(), device_handle, config_handle, event_code);
}

void UsbHostTask::RegisterUsbHostEventCallback(uint32_t vid, uint32_t pid,
                                               UsbHostEventCallback fn) {
  const uint32_t vidpid = vidpid_to_key(vid, pid);
  host_event_callbacks_[vidpid] = fn;

  const IRQn_Type irq_number = USB_OTG2_IRQn;
  // Highest possible prio for TPU stability.
  NVIC_SetPriority(irq_number, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
  NVIC_EnableIRQ(irq_number);
}

void UsbHostTask::TaskMain() {
  while (true) {
    USB_HostEhciTaskFunction(host_handle());
  }
}

void UsbHostTask::Init() {
  CHECK(xTaskCreate(UsbHostTask::StaticTaskMain, "UsbHostTask",
                    configMINIMAL_STACK_SIZE * 10, this, kUsbHostTaskPriority,
                    nullptr) == pdPASS);
}

UsbHostTask::UsbHostTask() {
  constexpr uint32_t usb_clock_freq = 24000000;
  usb_phy_config_struct_t phyConfig = {
      BOARD_USB_PHY_D_CAL,
      BOARD_USB_PHY_TXCAL45DP,
      BOARD_USB_PHY_TXCAL45DM,
  };

  CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, usb_clock_freq);
  CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, usb_clock_freq);
  USB_EhciLowPowerPhyInit(kUSBControllerId, BOARD_XTAL0_CLK_HZ, &phyConfig);

  const usb_status_t status =
      USB_HostInit(kUSBControllerId, &host_handle_, USB_HostEvent);
  if (status != kStatus_USB_Success) {
    printf("USB_HostInit failed\r\n");
    return;
  }

#if 0
    uint32_t usb_version;
    USB_HostGetVersion(&usb_version);
    printf("USB host stack version: %lu.%lu.%lu\r\n", (usb_version >> 16) & 0xFF, (usb_version >> 8) & 0xFF, usb_version & 0xFF);
#endif
}

}  // namespace coralmicro
