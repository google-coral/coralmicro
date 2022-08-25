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

#ifndef __LIBS_MSC_UMS_MSC_UMS_H_
#define __LIBS_MSC_UMS_MSC_UMS_H_

#include <map>

/* clang-format off */
#include "libs/usb/descriptors.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"  // Must be above other class headers.
#include "third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/usb_examples/usb_device_msc_disk/freertos/cm7/usb_device_msc.h"
/* clang-format on */

namespace coralmicro {

class MscUms {
 public:
  MscUms();
  MscUms(const MscUms &) = delete;
  MscUms &operator=(const MscUms &) = delete;
  void Init(uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t data_iface);
  usb_device_class_config_struct_t &config_data() { return config_; }
  void *descriptor_data() { return &descriptor_; }
  size_t descriptor_data_size() { return sizeof(descriptor_); }
  void SetClassHandle(class_handle_t class_handle);
  bool HandleEvent(uint32_t event, void *param);

 private:
  static usb_status_t Handler(class_handle_t class_handle, uint32_t event,
                              void *param);
  usb_status_t Handler(uint32_t event, void *param);
  usb_device_endpoint_struct_t msc_ums_data_endpoints_[2] = {
      {
          0,  // set in Init
          USB_ENDPOINT_BULK,
          512,
      },
      {
          0,  // set in Init
          USB_ENDPOINT_BULK,
          512,
      },
  };
  usb_device_interface_struct_t msc_ums_data_interface_[1] = {
      {0,
       {
           ARRAY_SIZE(msc_ums_data_endpoints_),
           msc_ums_data_endpoints_,
       }},
  };
  usb_device_interfaces_struct_t msc_ums_interfaces_[1] = {
      {
          0x08,  // InterfaceClass
          0x06,  // InterfaceSubClass
          0x50,  // InterfaceProtocol
          0,     // set in Init
          msc_ums_data_interface_,
          ARRAY_SIZE(msc_ums_data_interface_),
      },
  };
  usb_device_interface_list_t msc_ums_interface_list_[1] = {
      {
          ARRAY_SIZE(msc_ums_interfaces_),
          msc_ums_interfaces_,
      },
  };
  usb_device_class_struct_t class_struct_{
      msc_ums_interface_list_,
      kUSB_DeviceClassTypeMsc,
      ARRAY_SIZE(msc_ums_interface_list_),
  };
  usb_device_class_config_struct_t config_{
      Handler,  // USB_DeviceMscCallback
      nullptr,
      &class_struct_,
  };
  MscUmsClassDescriptor descriptor_ = {
      {
          sizeof(InterfaceDescriptor),
          0x4,
          3,
          0,
          2,
          0x8,
          0x6,
          0x50,
          0,
      },  // InterfaceDescriptor
      {sizeof(EndpointDescriptor), 0x05,
       0x04 | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), 0x02,
       512, 0},  // EndpointDescriptor
      {sizeof(EndpointDescriptor), 0x05,
       0x05 | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
       0x02, 512, 0},  // EndpointDescriptor
  };                   // MscUmsClassDescriptor
  uint8_t bulk_in_ep_, bulk_out_ep_;
  class_handle_t class_handle_;

  static std::map<class_handle_t, MscUms *> handle_map_;
};

}  // namespace coralmicro

#endif  // __LIBS_MSC_UMS_MSC_UMS_H_
