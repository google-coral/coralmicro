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

#include "libs/usb/usb_device_task.h"

#include <cstdio>

#include "libs/base/utils.h"
#include "libs/nxp/rt1176-sdk/clock_config.h"
#include "third_party/modified/nxp/rt1176-sdk/board.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.h"

extern "C" void USB_OTG1_IRQHandler(void) {
  USB_DeviceEhciIsrFunction(
      coralmicro::UsbDeviceTask::GetSingleton()->device_handle());
}

namespace coralmicro {
namespace {
constexpr int kUSBControllerId = kUSB_ControllerEhci0;

// Super-basic unicode "conversion" function (no conversion really, just
// ascii into 2-byte expansion).
//
// Expects param->buffer to be set to a suitable output buffer
void ToUsbStringDescriptor(const char* s,
                           usb_device_get_string_descriptor_struct_t* param) {
  uint8_t* output = param->buffer;

  // prefix is size and type
  *output++ = 2 + 2 * strlen(s);
  *output++ = USB_DESCRIPTOR_TYPE_STRING;
  param->length = 2;

  while (*s) {
    *output++ = *s++;
    *output++ = 0;
    param->length += 2;
  }
}
}  // namespace

usb_status_t UsbDeviceTask::Handler(usb_device_handle device_handle,
                                    uint32_t event, void* param) {
  static uint8_t string_buffer[64];
  bool event_ret = true;
  usb_status_t ret = kStatus_USB_Error;
  switch (event) {
    case kUSB_DeviceEventBusReset:
      ret = kStatus_USB_Success;
      break;
    case kUSB_DeviceEventGetDeviceDescriptor: {
      auto* device_desc =
          static_cast<usb_device_get_device_descriptor_struct_t*>(param);
      device_desc->buffer = const_cast<uint8_t*>(
          reinterpret_cast<const uint8_t*>(&device_descriptor_));
      device_desc->length = sizeof(device_descriptor_);
      ret = kStatus_USB_Success;
      break;
    }
    case kUSB_DeviceEventGetConfigurationDescriptor: {
      auto* config_desc =
          static_cast<usb_device_get_configuration_descriptor_struct_t*>(param);
      config_desc->buffer = composite_descriptor_.data();
      config_desc->length = composite_descriptor_.size();
      ret = kStatus_USB_Success;
      break;
    }
    case kUSB_DeviceEventGetStringDescriptor: {
      auto* string_desc =
          static_cast<usb_device_get_string_descriptor_struct_t*>(param);
      string_desc->buffer = string_buffer;
      if (string_desc->stringIndex == 0) {
        string_desc->length = sizeof(lang_id_desc_);
        memcpy(string_desc->buffer, &lang_id_desc_, sizeof(lang_id_desc_));
        ret = kStatus_USB_Success;
        break;
      }
      if (string_desc->languageId != 0x0409) {  // English
        ret = kStatus_USB_InvalidRequest;
        break;
      }
      switch (string_desc->stringIndex) {
        case 1:
          ToUsbStringDescriptor("Google", string_desc);
          ret = kStatus_USB_Success;
          break;
        case 2:
          ToUsbStringDescriptor("Coral Dev Board Micro", string_desc);
          ret = kStatus_USB_Success;
          break;
        case 3:
          ToUsbStringDescriptor(serial_number_.c_str(), string_desc);
          ret = kStatus_USB_Success;
          break;
        default:
          printf("Unhandled string request: %d\r\n", string_desc->stringIndex);
          ret = kStatus_USB_InvalidRequest;
          break;
      }
      break;
    }
    case kUSB_DeviceEventSetConfiguration:
      for (size_t i = 0; i < set_handle_callbacks_.size(); i++) {
        set_handle_callbacks_[i](configs_[i].classHandle);
      }
      for (size_t i = 0; i < handle_event_callbacks_.size(); i++) {
        event_ret &= handle_event_callbacks_[i](event, param);
      }
      if (event_ret) {
        ret = kStatus_USB_Success;
      }
      break;
    case kUSB_DeviceEventSetInterface:
    case kUSB_DeviceEventGetHidReportDescriptor:
      ret = kStatus_USB_Success;
      for (size_t i = 0; i < handle_event_callbacks_.size(); i++) {
        event_ret &= handle_event_callbacks_[i](event, param);
      }
      if (event_ret) {
        ret = kStatus_USB_Success;
      }
      break;
#if !defined(ELFLOADER)
    case kUSB_DeviceEventSuspend:
      USB_DeviceSetStatus(device_handle_, kUSB_DeviceStatusBusSuspend, nullptr);
      ret = kStatus_USB_Success;
      break;
    case kUSB_DeviceEventResume:
      USB_DeviceSetStatus(device_handle_, kUSB_DeviceStatusBusResume, nullptr);
      ret = kStatus_USB_Success;
      break;
#endif
    default:
      printf("%s event unhandled 0x%lx\r\n", __func__, event);
  }
  return ret;
}

void UsbDeviceTask::AddDevice(const usb_device_class_config_struct_t& config,
                              UsbSetHandleCallback sh_cb,
                              UsbHandleEventCallback he_cb,
                              const void* descriptor_data,
                              size_t descriptor_data_size) {
  configs_.push_back(config);
  set_handle_callbacks_.push_back(sh_cb);
  handle_event_callbacks_.push_back(he_cb);

  size_t old_size = composite_descriptor_size_;
  composite_descriptor_size_ += descriptor_data_size;
  memcpy(composite_descriptor_.data() + old_size, descriptor_data,
         descriptor_data_size);

  auto* p_composite_descriptor =
      reinterpret_cast<CompositeDescriptor*>(composite_descriptor_.data());
  p_composite_descriptor->conf.total_length = composite_descriptor_size_;
}

bool UsbDeviceTask::Init() {
  config_list_ = {configs_.data(), &UsbDeviceTask::StaticHandler,
                  static_cast<uint8_t>(configs_.size())};
  usb_status_t ret =
      USB_DeviceClassInit(kUSBControllerId, &config_list_, &device_handle_);
  if (ret != kStatus_USB_Success) {
    printf("USB_DeviceClassInit failed %d\r\n", ret);
    return false;
  }
  SDK_DelayAtLeastUs(50000, CLOCK_GetFreq(kCLOCK_CpuClk));
  ret = USB_DeviceRun(device_handle_);
  if (ret != kStatus_USB_Success) {
    printf("USB_DeviceRun failed\r\n");
    return false;
  }
  return true;
}

UsbDeviceTask::UsbDeviceTask() {
  serial_number_ = coralmicro::GetSerialNumber();

  constexpr uint32_t usb_clock_freq = 24000000;
  usb_phy_config_struct_t phy_config = {
      BOARD_USB_PHY_D_CAL,
      BOARD_USB_PHY_TXCAL45DP,
      BOARD_USB_PHY_TXCAL45DM,
  };

  CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usb_clock_freq);
  CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usb_clock_freq);
  USB_EhciLowPowerPhyInit(kUSBControllerId, BOARD_XTAL0_CLK_HZ, &phy_config);

  constexpr IRQn_Type irq_number = USB_OTG1_IRQn;
  NVIC_SetPriority(irq_number, 6);
  NVIC_EnableIRQ(irq_number);
  constexpr CompositeDescriptor composite_descriptor = {
      .conf =
          {
              .length = sizeof(ConfigurationDescriptor),
              .descriptor_type = 0x02,
              .total_length = sizeof(CompositeDescriptor),
              .num_interfaces = 0,  // Managed by next_interface_value
              .config_value = 1,
              .config = 0,
              .attributes = 0x80,  // kUsb11AndHigher
              .max_power = 250,    // kUses500mA
          },
  };
  composite_descriptor_size_ = sizeof(composite_descriptor);
  memcpy(composite_descriptor_.data(), &composite_descriptor,
         sizeof(composite_descriptor));
}

void UsbDeviceTask::UsbDeviceTaskFn() {
  USB_DeviceEhciTaskFunction(device_handle_);
}

}  // namespace coralmicro
