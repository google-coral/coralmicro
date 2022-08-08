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

#ifndef LIBS_CDC_ACM_CDC_ACM_H_
#define LIBS_CDC_ACM_CDC_ACM_H_

#include <functional>
#include <map>

/* clang-format off */
#include "libs/usb/descriptors.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"  // Must be above other class headers.
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.h"
/* clang-format on */

typedef struct _usb_host_cdc_line_coding_struct {
  uint32_t dwDTERate;  /*!< Data terminal rate, in bits per second*/
  uint8_t bCharFormat; /*!< Stop bits*/
  uint8_t bParityType; /*!< Parity*/
  uint8_t bDataBits;   /*!< Data bits (5, 6, 7, 8 or 16).*/
} __attribute__((packed)) usb_host_cdc_line_coding_struct_t;

namespace coralmicro {

class CdcAcm {
 public:
  using RxHandler = std::function<void(const uint8_t *, const uint32_t)>;

  CdcAcm() = default;
  CdcAcm(const CdcAcm &) = delete;
  CdcAcm &operator=(const CdcAcm &) = delete;
  void Init(uint8_t interrupt_in_ep, uint8_t bulk_in_ep, uint8_t bulk_out_ep,
            uint8_t comm_iface, uint8_t data_iface, RxHandler rx_handler);
  const usb_device_class_config_struct_t &config_data() const {
    return config_;
  }
  const void *descriptor_data() const { return &descriptor_; }
  size_t descriptor_data_size() const { return sizeof(descriptor_); }
  // TODO(atv): Make me private
  void SetClassHandle(class_handle_t class_handle) {
    handle_map_[class_handle] = this;
    class_handle_ = class_handle;
  }
  bool HandleEvent(uint32_t event, void *param);
  bool Transmit(const uint8_t *buffer, const size_t length);

 private:
  bool can_transmit_ = false;
  void SetConfiguration();
  usb_status_t SetControlLineState(
      usb_device_cdc_acm_request_param_struct_t *acm_param);

  static std::map<class_handle_t, CdcAcm *> handle_map_;
  static usb_status_t StaticHandler(class_handle_t class_handle, uint32_t event,
                                    void *param) {
    return handle_map_[class_handle]->Handler(event, param);
  }
  usb_status_t Handler(uint32_t event, void *param);

  usb_device_endpoint_struct_t cdc_acm_comm_endpoints_[1] = {
      {
          .endpointAddress = 0,  // set in constructor
          .transferType = USB_ENDPOINT_INTERRUPT,
          .maxPacketSize = 8,
          .interval = 0,
      },
  };
  usb_device_endpoint_struct_t cdc_acm_data_endpoints_[2] = {
      {
          .endpointAddress = 0,  // set in constructor
          .transferType = USB_ENDPOINT_BULK,
          .maxPacketSize = 512,
          .interval = 0,
      },
      {
          .endpointAddress = 0,  // set in constructor
          .transferType = USB_ENDPOINT_BULK,
          .maxPacketSize = 512,
          .interval = 0,
      },
  };
  usb_device_interface_struct_t cdc_acm_comm_interface_[1] = {
      {
          .alternateSetting = 0,
          .endpointList =
              {
                  ARRAY_SIZE(cdc_acm_comm_endpoints_),
                  cdc_acm_comm_endpoints_,
              },
          .classSpecific = nullptr,
      },
  };
  usb_device_interface_struct_t cdc_acm_data_interface_[1] = {
      {
          .alternateSetting = 0,
          .endpointList =
              {
                  ARRAY_SIZE(cdc_acm_data_endpoints_),
                  cdc_acm_data_endpoints_,
              },
          .classSpecific = nullptr,
      },
  };
  usb_device_interfaces_struct_t cdc_acm_interfaces_[2] = {
      // Comm
      {
          .classCode = 0x02,
          .subclassCode = 0x02,
          .protocolCode = 0x00,
          .interfaceNumber = 0,  // set in constructor
          .interface = cdc_acm_comm_interface_,
          .count = ARRAY_SIZE(cdc_acm_comm_interface_),
      },
      // Data
      {
          .classCode = 0x0A,
          .subclassCode = 0x00,
          .protocolCode = 0x00,
          .interfaceNumber = 0,  // set in constructor
          .interface = cdc_acm_data_interface_,
          .count = ARRAY_SIZE(cdc_acm_data_interface_),
      },
  };
  usb_device_interface_list_t cdc_acm_interface_list_[1] = {
      {
          .count = ARRAY_SIZE(cdc_acm_interfaces_),
          .interfaces = cdc_acm_interfaces_,
      },
  };
  usb_device_class_struct_t class_struct_{
      .interfaceList = cdc_acm_interface_list_,
      .type = kUSB_DeviceClassTypeCdc,
      .configurations = ARRAY_SIZE(cdc_acm_interface_list_),
  };
  usb_device_class_config_struct_t config_{
      .classCallback = StaticHandler,
      .classHandle = nullptr,
      .classInfomation = &class_struct_,
  };
  static constexpr CdcAcmClassDescriptor descriptor_ = {
      .iad0 =
          {
              .length = sizeof(InterfaceAssociationDescriptor),
              .descriptor_type = 0x0B,
              .first_interface = 0,
              .interface_count = 2,
              .function_class = 0x02,
              .function_subclass = 0x02,
              .function_protocol = 0x01,
              .interface = 0,
          },
      .cmd_iface =
          {
              .length = sizeof(InterfaceDescriptor),
              .descriptor_type = 0x04,
              .interface_number = 0,
              .alternate_setting = 0,
              .num_endpoints = 1,
              .interface_class = 0x02,
              .interface_subclass = 0x02,
              .interface_protocol = 0x01,
              .interface = 0,
          },
      .cmd_hdr_fd =
          {
              .length = sizeof(CdcHeaderFunctionalDescriptor),
              .descriptor_type = 0x24,
              .descriptor_subtype = 0x00,
              .cdc = 0x0110,
          },
      .cmd_mgmt_fd =
          {
              .function_length = sizeof(CdcCallManagementFunctionalDescriptor),
              .descriptor_type = 0x24,
              .descriptor_subtype = 0x01,
              .capabilities = 0,
              .data_interface = 1,
          },
      .cmd_acm_fd =
          {
              .function_length = sizeof(CdcAcmFunctionalDescriptor),
              .descriptor_type = 0x24,
              .descriptor_subtype = 0x02,
              .capabilities = 2,
          },
      .cmd_union_fd =
          {
              .function_length = sizeof(CdcUnionFunctionalDescriptor),
              .descriptor_type = 0x24,
              .descriptor_subtype = 0x06,
              .controller_iface = 0,
              .peripheral_iface0 = 1,
          },
      .cmd_ep =
          {
              .length = sizeof(EndpointDescriptor),
              .descriptor_type = 0x05,
              .endpoint_address = 1 | 0x80,
              .attributes = 0x03,
              .max_packet_size = 10,
              .interval = 9,
          },
      .data_iface =
          {
              .length = sizeof(InterfaceDescriptor),
              .descriptor_type = 0x04,
              .interface_number = 1,
              .alternate_setting = 0,
              .num_endpoints = 2,
              .interface_class = 0x0A,
              .interface_subclass = 0x00,
              .interface_protocol = 0x00,
              .interface = 0,
          },
      .in_ep =
          {
              .length = sizeof(EndpointDescriptor),
              .descriptor_type = 0x05,
              .endpoint_address = 2 | 0x80,
              .attributes = 0x02,
              .max_packet_size = 512,
              .interval = 0,
          },
      .out_ep =
          {
              .length = sizeof(EndpointDescriptor),
              .descriptor_type = 0x05,
              .endpoint_address = 3 & 0x7F,
              .attributes = 0x02,
              .max_packet_size = 512,
              .interval = 0,
          },
  };

  uint8_t tx_buffer_[512];
  uint8_t rx_buffer_[512];
  uint8_t serial_state_buffer_[10];
  SemaphoreHandle_t tx_semaphore_;
  uint8_t interrupt_in_ep_, bulk_in_ep_, bulk_out_ep_;
  RxHandler rx_handler_;
  class_handle_t class_handle_;
  usb_host_cdc_line_coding_struct_t line_coding_;
};

}  // namespace coralmicro

#endif  // LIBS_CDC_ACM_CDC_ACM_H_
