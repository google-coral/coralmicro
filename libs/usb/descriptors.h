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

#ifndef LIBS_USB_DESCRIPTORS_H_
#define LIBS_USB_DESCRIPTORS_H_

#include <cstdint>

namespace coralmicro {

struct DeviceDescriptor {
  uint8_t length;
  uint8_t descriptor_type;
  uint16_t bcd_usb;
  uint8_t device_class;
  uint8_t device_subclass;
  uint8_t device_protocol;
  uint8_t max_packet_size;
  uint16_t id_vendor;
  uint16_t id_product;
  uint16_t bcd_device;
  uint8_t manufacturer;
  uint8_t product;
  uint8_t serial_number;
  uint8_t num_configurations;
} __attribute__((packed));

struct ConfigurationDescriptor {
  uint8_t length;
  uint8_t descriptor_type;
  uint16_t total_length;
  uint8_t num_interfaces;
  uint8_t config_value;
  uint8_t config;
  uint8_t attributes;
  uint8_t max_power;
} __attribute__((packed));

struct InterfaceAssociationDescriptor {
  uint8_t length;
  uint8_t descriptor_type;
  uint8_t first_interface;
  uint8_t interface_count;
  uint8_t function_class;
  uint8_t function_subclass;
  uint8_t function_protocol;
  uint8_t interface;
} __attribute__((packed));

struct InterfaceDescriptor {
  uint8_t length;
  uint8_t descriptor_type;
  uint8_t interface_number;
  uint8_t alternate_setting;
  uint8_t num_endpoints;
  uint8_t interface_class;
  uint8_t interface_subclass;
  uint8_t interface_protocol;
  uint8_t interface;
} __attribute__((packed));

struct EndpointDescriptor {
  uint8_t length;
  uint8_t descriptor_type;
  uint8_t endpoint_address;
  uint8_t attributes;
  uint16_t max_packet_size;
  uint8_t interval;
} __attribute__((packed));

struct CdcHeaderFunctionalDescriptor {
  uint8_t length;
  uint8_t descriptor_type;
  uint8_t descriptor_subtype;
  uint16_t cdc;
} __attribute__((packed));

struct CdcCallManagementFunctionalDescriptor {
  uint8_t function_length;
  uint8_t descriptor_type;
  uint8_t descriptor_subtype;
  uint8_t capabilities;
  uint8_t data_interface;
} __attribute__((packed));

struct CdcAcmFunctionalDescriptor {
  uint8_t function_length;
  uint8_t descriptor_type;
  uint8_t descriptor_subtype;
  uint8_t capabilities;
} __attribute__((packed));

struct CdcUnionFunctionalDescriptor {
  uint8_t function_length;
  uint8_t descriptor_type;
  uint8_t descriptor_subtype;
  uint8_t controller_iface;
  uint8_t peripheral_iface0;
} __attribute__((packed));

struct CdcAcmClassDescriptor {
  InterfaceAssociationDescriptor iad0;
  // Command
  InterfaceDescriptor cmd_iface;
  CdcHeaderFunctionalDescriptor cmd_hdr_fd;
  CdcCallManagementFunctionalDescriptor cmd_mgmt_fd;
  CdcAcmFunctionalDescriptor cmd_acm_fd;
  CdcUnionFunctionalDescriptor cmd_union_fd;
  EndpointDescriptor cmd_ep;
  // Data
  InterfaceDescriptor data_iface;
  EndpointDescriptor in_ep;
  EndpointDescriptor out_ep;
} __attribute__((packed));

struct CdcEemClassDescriptor {
  InterfaceDescriptor iface;
  EndpointDescriptor in_ep;
  EndpointDescriptor out_ep;
} __attribute__((packed));

struct MscUmsClassDescriptor {
  InterfaceDescriptor iface;
  EndpointDescriptor in_ep;
  EndpointDescriptor out_ep;
} __attribute__((packed));

struct HidDescriptor {
  uint8_t length;
  uint8_t type;
  uint16_t bcd_hid;
  uint8_t country_code;
  uint8_t num_descriptors;
  uint8_t descriptor_type;
  uint16_t descriptor_len;
} __attribute__((packed));

struct HidClassDescriptor {
  InterfaceDescriptor iface;
  HidDescriptor hid;
  EndpointDescriptor in_ep;
  EndpointDescriptor out_ep;
} __attribute__((packed));

struct CompositeDescriptor {
  ConfigurationDescriptor conf;
} __attribute__((packed));

struct LangIdDescriptor {
  uint8_t length;
  uint8_t descriptor_type;
  uint16_t lang_id;
} __attribute__((packed));

}  // namespace coralmicro

#endif  // LIBS_USB_DESCRIPTORS_H_
