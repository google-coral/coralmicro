// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "apps/elf_loader/elf_loader.h"

uint8_t elfloader_hid_report[] = {
    0x05, 0x81, /* Usage Page (Vendor defined) */
    0x09, 0x82, /* Usage (Vendor defined) */
    0xa1, 0x01, /* Collection (Application) */
    0x09, 0x83, /* Usage (Vendor defined) */

    0x09, 0x84, /* Usage (Vendor defined) */
    0x15, 0x80, /* Logical Minimum (-128) */
    0x25, 0x7f, /* Logical Maximum (127) */
    0x75, 0x08, /* Report Size (8U bits) */
    0x95, 0x40, /* Report count (64U bytes) */
    0x81, 0x02, /* Input(Data, Variable, Absolute) */

    0x09, 0x84, /* Usage (Vendor defined) */
    0x15, 0x80, /* Logical Minimum (-128) */
    0x25, 0x7f, /* Logical Maximum (127) */
    0x75, 0x08, /* Report Size (8U bits) */
    0x95, 0x40, /* Report Count (64U) */
    0x91, 0x02, /* Output (Data, Variable, Absolute) */
    0xc0        /* End collection */
};
uint16_t elfloader_hid_report_size = sizeof(elfloader_hid_report);

coralmicro::HidClassDescriptor elfloader_descriptor_data = {
    {sizeof(coralmicro::InterfaceDescriptor), 0x4, 0, 0, 2, 3, 0, 0,
     0},  // InterfaceDescriptor
    {
        sizeof(coralmicro::HidDescriptor),
        33,
        0x0100,
        0,
        1,
        34,
        elfloader_hid_report_size,
    },  // HidDescriptor
    {
        sizeof(coralmicro::EndpointDescriptor),
        5,
        0 /* set by code */,
        0x03,
        512,
        3,
    },  // EndpointDescriptor
    {
        sizeof(coralmicro::EndpointDescriptor),
        5,
        0 /* set by code */,
        0x03,
        512,
        3,
    },  // EndpointDescriptor
};

usb_device_endpoint_struct_t elfloader_hid_endpoints[2] = {
    {
        0,  // in
        USB_ENDPOINT_INTERRUPT,
        128,
    },
    {
        0,  // out
        USB_ENDPOINT_INTERRUPT,
        128,
    }};

usb_device_interface_struct_t elfloader_hid_interface[1] = {
    {
        0,
        {
            ARRAY_SIZE(elfloader_hid_endpoints),
            elfloader_hid_endpoints,
        },
    },
};

usb_device_interfaces_struct_t elfloader_interfaces[1] = {
    {
        0x03,
        0x00,
        0x00,
        0,
        elfloader_hid_interface,
        ARRAY_SIZE(elfloader_hid_interface),
    },
};

usb_device_interface_list_t elfloader_interface_list[1] = {
    ARRAY_SIZE(elfloader_interfaces),
    elfloader_interfaces,
};

usb_device_class_struct_t elfloader_class_struct = {
    elfloader_interface_list,
    kUSB_DeviceClassTypeHid,
    ARRAY_SIZE(elfloader_interface_list),
};

usb_device_class_config_struct_t elfloader_config_data = {
    nullptr,
    nullptr,
    &elfloader_class_struct,
};
