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

#ifndef APPS_ELFLOADER_ELF_LOADER_H_
#define APPS_ELFLOADER_ELF_LOADER_H_

#include <cstddef>
#include <cstdint>

#include "libs/usb/descriptors.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_hid.h"

enum class ElfloaderCommand : uint8_t {
  kSetSize = 0,
  kBytes = 1,
  kDone = 2,
  kResetToBootloader = 3,
  kTarget = 4,
  kResetToFlash = 5,
  kFormat = 6,
};

enum class ElfloaderTarget : uint8_t {
  kRam = 0,
  kPath = 1,
  kFilesystem = 2,
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
extern coralmicro::HidClassDescriptor elfloader_descriptor_data;
extern usb_device_endpoint_struct_t elfloader_hid_endpoints[];
extern usb_device_interface_struct_t elfloader_hid_interface[];
extern usb_device_interfaces_struct_t elfloader_interfaces[];
extern usb_device_interface_list_t elfloader_interface_list[];
extern usb_device_class_struct_t elfloader_class_struct;
extern usb_device_class_config_struct_t elfloader_config_data;

#endif  // APPS_ELFLOADER_ELF_LOADER_H_
