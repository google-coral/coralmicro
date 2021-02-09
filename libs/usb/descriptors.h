#ifndef __LIBS_USB_DESCRIPTORS_H__
#define __LIBS_USB_DESCRIPTORS_H__

namespace valiant {

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

struct CompositeDescriptor {
    ConfigurationDescriptor conf;
    InterfaceAssociationDescriptor iad0;
    CdcAcmClassDescriptor iface0;
    CdcEemClassDescriptor iface1;
} __attribute__((packed));

struct LangIdDescriptor {
    uint8_t length;
    uint8_t descriptor_type;
    uint16_t lang_id;
} __attribute__((packed));

}  // namespace valiant

#endif  // __LIBS_USB_DESCRIPTORS_H__
