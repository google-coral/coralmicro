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

#include <elf.h>

#include <memory>

#include "libs/base/filesystem.h"
#include "libs/base/reset.h"
#include "libs/base/tasks.h"
#include "libs/nxp/rt1176-sdk/board_hardware.h"
#include "libs/usb/descriptors.h"
#include "libs/usb/usb_device_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_soc_src.h"

extern "C" uint32_t disable_usb_timeout;
uint32_t disable_usb_timeout __attribute__((section(".noinit_rpmsg_sh_mem")));

extern "C" uint32_t vPortGetRunTimeCounterValue(void) { return 0; }

namespace {
using coralmicro::Lfs;
void elfloader_main(void *param);

TimerHandle_t usb_timer;
uint8_t *elfloader_recv_image = nullptr;
char *elfloader_recv_path = nullptr;
uint8_t elfloader_data[64];
class_handle_t elfloader_class_handle;
ElfloaderTarget elfloader_target = ElfloaderTarget::kRam;
lfs_file_t file_handle;
bool filesystem_formatted = false;

void elfloader_recv(const uint8_t *buffer, uint32_t length) {
  ElfloaderCommand cmd = static_cast<ElfloaderCommand>(buffer[0]);
  const ElfloaderSetSize *set_size =
      reinterpret_cast<const ElfloaderSetSize *>(&buffer[1]);
  const ElfloaderBytes *bytes =
      reinterpret_cast<const ElfloaderBytes *>(&buffer[1]);
  const ElfloaderTarget *target =
      reinterpret_cast<const ElfloaderTarget *>(&buffer[1]);
  xTimerStop(usb_timer, 0);
  switch (cmd) {
    case ElfloaderCommand::kSetSize:
      assert(length >= sizeof(ElfloaderSetSize) + 1);
      switch (elfloader_target) {
        case ElfloaderTarget::kFilesystem:
          break;
        case ElfloaderTarget::kRam:
          elfloader_recv_image = new uint8_t[set_size->size];
          break;
        case ElfloaderTarget::kPath:
          elfloader_recv_path = static_cast<char *>(malloc(set_size->size + 1));
          memset(elfloader_recv_path, 0, set_size->size + 1);
          break;
      }
      break;
    case ElfloaderCommand::kBytes:
      assert(length >= sizeof(ElfloaderBytes) + 1);
      switch (elfloader_target) {
        case ElfloaderTarget::kRam:
          memcpy(elfloader_recv_image + bytes->offset,
                 &buffer[1] + sizeof(ElfloaderBytes), bytes->size);
          break;
        case ElfloaderTarget::kPath:
          memcpy(elfloader_recv_path + bytes->offset,
                 &buffer[1] + sizeof(ElfloaderBytes), bytes->size);
          break;
        case ElfloaderTarget::kFilesystem:
          lfs_file_write(Lfs(), &file_handle,
                         &buffer[1] + sizeof(ElfloaderBytes), bytes->size);
          break;
      }
      break;
    case ElfloaderCommand::kDone:
      switch (elfloader_target) {
        case ElfloaderTarget::kRam:
          xTaskCreate(elfloader_main, "elfloader_main",
                      configMINIMAL_STACK_SIZE * 10, elfloader_recv_image,
                      coralmicro::kAppTaskPriority, nullptr);
          break;
        case ElfloaderTarget::kPath: {
          // TODO(atv): This stuff can fail. We should propagate errors back to
          // the Python side if possible.
          auto dir = coralmicro::LfsDirname(elfloader_recv_path);
          coralmicro::LfsMakeDirs(dir.c_str());
          lfs_file_open(Lfs(), &file_handle, elfloader_recv_path,
                        LFS_O_TRUNC | LFS_O_CREAT | LFS_O_RDWR);
          free(elfloader_recv_path);
          elfloader_recv_path = nullptr;
        } break;
        case ElfloaderTarget::kFilesystem:
          lfs_file_close(Lfs(), &file_handle);
          break;
      }
      break;
    case ElfloaderCommand::kResetToBootloader:
      coralmicro::ResetToBootloader();
      break;
    case ElfloaderCommand::kResetToFlash:
      coralmicro::ResetToFlash();
      break;
    case ElfloaderCommand::kTarget:
      elfloader_target = *target;
      break;
    case ElfloaderCommand::kFormat:
      coralmicro::LfsInit(/*force_format=*/true);
      filesystem_formatted = true;
      break;
  }
}

bool elfloader_HandleEvent(uint32_t event, void *param) {
  bool ret = true;
  usb_device_get_hid_descriptor_struct_t *get_hid_descriptor =
      static_cast<usb_device_get_hid_descriptor_struct_t *>(param);
  switch (event) {
    case kUSB_DeviceEventSetConfiguration:
      USB_DeviceHidRecv(elfloader_class_handle,
                        elfloader_hid_endpoints[kRxEndpoint].endpointAddress,
                        elfloader_data, sizeof(elfloader_data));
      break;
    case kUSB_DeviceEventGetHidReportDescriptor:
      get_hid_descriptor->buffer = elfloader_hid_report;
      get_hid_descriptor->length = elfloader_hid_report_size;
      get_hid_descriptor->interfaceNumber =
          elfloader_interfaces[0].interfaceNumber;
      break;
    default:
      ret = false;
      break;
  }
  return ret;
}

void elfloader_SetClassHandle(class_handle_t class_handle) {
  elfloader_class_handle = class_handle;
}

usb_status_t elfloader_Handler(class_handle_t class_handle, uint32_t event,
                               void *param) {
  uint8_t dummy = 0;
  usb_status_t ret = kStatus_USB_Success;
  usb_device_endpoint_callback_message_struct_t *message =
      static_cast<usb_device_endpoint_callback_message_struct_t *>(param);
  switch (event) {
    case kUSB_DeviceHidEventRecvResponse:
      if (message->length != USB_UNINITIALIZED_VAL_32) {
        elfloader_recv(message->buffer, message->length);
      }
      USB_DeviceHidSend(elfloader_class_handle,
                        elfloader_hid_endpoints[kTxEndpoint].endpointAddress,
                        &dummy, 1);
      USB_DeviceHidRecv(elfloader_class_handle,
                        elfloader_hid_endpoints[kRxEndpoint].endpointAddress,
                        elfloader_data, sizeof(elfloader_data));
      break;
    case kUSB_DeviceHidEventGetReport:
      ret = kStatus_USB_InvalidRequest;
      break;
    case kUSB_DeviceHidEventSendResponse:
    case kUSB_DeviceHidEventSetIdle:
      break;
    default:
      ret = kStatus_USB_Error;
      break;
  }
  return ret;
}

usb_device_class_config_struct_t elfloader_config_data_ = {
    elfloader_Handler,
    nullptr,
    &elfloader_class_struct,
};

typedef void (*entry_point)(void);
void elfloader_main(void *param) {
  ssize_t elf_size = -1;
  std::unique_ptr<uint8_t[]> application_elf;
  if (!param) {
    elf_size = coralmicro::LfsSize("/default.elf");
    if (elf_size != -1) {
      application_elf = std::make_unique<uint8_t[]>(elf_size);
      if (coralmicro::LfsReadFile("/default.elf", application_elf.get(),
                                  elf_size) != static_cast<size_t>(elf_size)) {
        application_elf.reset();
      }
    }
  } else {
    application_elf.reset(reinterpret_cast<uint8_t *>(param));
  }

  // If we do not have an application for any reason, suspend this thread.
  // Otherwise, suspend the USB thread so that the host cannot try to talk to us
  // and cause confusion.
  if (!application_elf || (!param && elf_size == -1)) {
    vTaskSuspend(nullptr);
  } else {
    vTaskSuspend(reinterpret_cast<TaskHandle_t>(pvTimerGetTimerID(usb_timer)));
  }

  Elf32_Ehdr *elf_header =
      reinterpret_cast<Elf32_Ehdr *>(application_elf.get());
  assert(EF_ARM_EABI_VERSION(elf_header->e_flags) == EF_ARM_EABI_VER5);
  assert(elf_header->e_phentsize == sizeof(Elf32_Phdr));

  for (int i = 0; i < elf_header->e_phnum; ++i) {
    Elf32_Phdr *program_header = reinterpret_cast<Elf32_Phdr *>(
        application_elf.get() + elf_header->e_phoff + sizeof(Elf32_Phdr) * i);
    if (program_header->p_type != PT_LOAD) {
      continue;
    }
    if (program_header->p_filesz == 0) {
      continue;
    }
    memcpy(reinterpret_cast<void *>(program_header->p_paddr),
           application_elf.get() + program_header->p_offset,
           program_header->p_filesz);
  }

  entry_point entry_point_fn =
      reinterpret_cast<entry_point>(elf_header->e_entry);
  entry_point_fn();

  vTaskSuspend(nullptr);
}

void usb_device_task(void *param) {
  while (true) {
    coralmicro::UsbDeviceTask::GetSingleton()->UsbDeviceTaskFn();
    taskYIELD();
  }
}

void usb_timer_callback(TimerHandle_t timer) {
  xTaskCreate(elfloader_main, "elfloader_main", configMINIMAL_STACK_SIZE * 10,
              nullptr, coralmicro::kAppTaskPriority, nullptr);
}
}  // namespace

extern "C" int main(int argc, char **argv) {
  BOARD_InitHardware(false);
  coralmicro::LfsInit();

  TaskHandle_t usb_task;
  // Create a task that keeps the device from attempting to sleep.
  xTaskCreate([](void* param) { while(true) {} }, "wake task", configMINIMAL_STACK_SIZE, nullptr, 1U, nullptr);
  xTaskCreate(usb_device_task, "usb_device_task", configMINIMAL_STACK_SIZE * 10,
              nullptr, coralmicro::kUsbDeviceTaskPriority, &usb_task);
  usb_timer = xTimerCreate("usb_timer", pdMS_TO_TICKS(1000), pdFALSE, usb_task,
                           usb_timer_callback);

  // See TRM chapter 10.3.1 for boot mode choices.
  // If boot mode is *not* the serial downloader, start the boot timer.
  if (SRC_GetBootMode(SRC) != kSerialDownloader &&
      disable_usb_timeout != 0xffffffff) {
    xTimerStart(usb_timer, 0);
  }

  elfloader_hid_endpoints[0].endpointAddress =
      coralmicro::UsbDeviceTask::GetSingleton()->next_descriptor_value() |
      (USB_IN << 7);
  elfloader_hid_endpoints[1].endpointAddress =
      coralmicro::UsbDeviceTask::GetSingleton()->next_descriptor_value() |
      (USB_OUT << 7);
  elfloader_descriptor_data.in_ep.endpoint_address =
      elfloader_hid_endpoints[0].endpointAddress;
  elfloader_descriptor_data.out_ep.endpoint_address =
      elfloader_hid_endpoints[1].endpointAddress;
  elfloader_interfaces[0].interfaceNumber =
      coralmicro::UsbDeviceTask::GetSingleton()->next_interface_value();
  coralmicro::UsbDeviceTask::GetSingleton()->AddDevice(
      elfloader_config_data_, elfloader_SetClassHandle, elfloader_HandleEvent,
      &elfloader_descriptor_data, sizeof(elfloader_descriptor_data));

  coralmicro::UsbDeviceTask::GetSingleton()->Init();

  vTaskStartScheduler();
  return 0;
}
