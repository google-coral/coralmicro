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

#ifndef LIBS_USB_USB_DEVICE_TASK_H_
#define LIBS_USB_USB_DEVICE_TASK_H_

#include <array>
#include <functional>
#include <string>
#include <vector>

/* clang-format off */
#include "third_party/modified/nxp/rt1176-sdk/usb_device_config.h"  // must be before usb_device.h
#include "libs/usb/descriptors.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
/* clang-format on */

#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"

namespace coralmicro {

// Operates the Dev Board Micro as a USB device.
class UsbDeviceTask {
 public:
  using UsbSetHandleCallback = std::function<void(class_handle_t)>;
  using UsbHandleEventCallback = std::function<bool(uint32_t, void*)>;

  UsbDeviceTask();
  UsbDeviceTask(const UsbDeviceTask&) = delete;
  UsbDeviceTask& operator=(const UsbDeviceTask&) = delete;

  void AddDevice(const usb_device_class_config_struct_t& config,
                 UsbSetHandleCallback sh_cb, UsbHandleEventCallback he_cb,
                 const void* descriptor_data, size_t descriptor_data_size);
  // @cond Do not generate docs
  bool Init();
  // @endcond
  static UsbDeviceTask* GetSingleton() {
    static UsbDeviceTask task;
    return &task;
  }
  void UsbDeviceTaskFn();

  uint8_t next_descriptor_value() { return ++next_descriptor_value_; }
  uint8_t next_interface_value() {
    uint8_t next_interface = next_interface_value_;
    CompositeDescriptor* p_composite_descriptor =
        reinterpret_cast<CompositeDescriptor*>(composite_descriptor_.data());
    p_composite_descriptor->conf.num_interfaces = ++next_interface_value_;
    return next_interface;
  }
  usb_device_handle device_handle() const { return device_handle_; }

 private:
  uint8_t next_descriptor_value_ = 0;
  uint8_t next_interface_value_ = 0;

  static constexpr DeviceDescriptor device_descriptor_ = {
      .length = sizeof(DeviceDescriptor),
      .descriptor_type = 0x01,
      .bcd_usb = 0x0200,
      .device_class = 0xEF,
      .device_subclass = 0x02,
      .device_protocol = 0x01,
      .max_packet_size = 0x40,
      .id_vendor = 0x18d1,
#if defined(ELFLOADER)
      0x9307,
#else
      0x9308,
#endif
      .bcd_device = 0x0001,
      .manufacturer = 1,
      .product = 2,
      .serial_number = 3,
      .num_configurations = 1};
  std::array<uint8_t, 1024> composite_descriptor_;
  size_t composite_descriptor_size_ = 0;

  static constexpr LangIdDescriptor lang_id_desc_ = {
      .length = sizeof(LangIdDescriptor),
      .descriptor_type = 0x03,
      .lang_id = 0x0409,
  };

  static usb_status_t StaticHandler(usb_device_handle device_handle,
                                    uint32_t event, void* param) {
    return GetSingleton()->Handler(device_handle, event, param);
  }

  usb_status_t Handler(usb_device_handle device_handle, uint32_t event,
                       void* param);

  std::vector<usb_device_class_config_struct_t> configs_;
  usb_device_class_config_list_struct_t config_list_;
  usb_device_handle device_handle_;

  std::vector<UsbSetHandleCallback> set_handle_callbacks_;
  std::vector<UsbHandleEventCallback> handle_event_callbacks_;
  std::string serial_number_;
};

}  // namespace coralmicro

#endif  // LIBS_USB_USB_DEVICE_TASK_H_
