#ifndef _APPS_ELFLOADER_ELF_LOADER_H_
#define _APPS_ELFLOADER_ELF_LOADER_H_

#include "libs/usb/descriptors.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_hid.h"
#include <cstddef>
#include <cstdint>

enum class ElfloaderCommand : uint8_t {
    SetSize = 0,
    Bytes = 1,
    Done = 2,
    Reset = 3,
    Target = 4,
};

enum class ElfloaderTarget : uint8_t {
    Ram = 0,
    Path = 1,
    Filesystem = 2,
};

enum BootModes {
    kFuse = 0,
    kSerialDownloader = 1,
    kInternal = 2,
};

struct ElfloaderSetSize {
    size_t size;
} __attribute__((packed));

struct ElfloaderBytes {
    size_t size;
    size_t offset;
} __attribute__((packed));

constexpr int kTxEndpoint = 0;
constexpr int kRxEndpoint = 1;
extern uint8_t elfloader_hid_report[];
extern uint16_t elfloader_hid_report_size;
extern valiant::HidClassDescriptor elfloader_descriptor_data;
extern usb_device_endpoint_struct_t elfloader_hid_endpoints[];
extern usb_device_interface_struct_t elfloader_hid_interface[];
extern usb_device_interfaces_struct_t elfloader_interfaces[];
extern usb_device_interface_list_t elfloader_interface_list[];
extern usb_device_class_struct_t elfloader_class_struct;
extern usb_device_class_config_struct_t elfloader_config_data;

#endif  // _APPS_ELFLOADER_ELF_LOADER_H_
