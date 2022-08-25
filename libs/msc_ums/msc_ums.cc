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

#include "libs/msc_ums/msc_ums.h"

#include <cstring>

#include "fsl_cache.h"
#include "libs/base/check.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/components/flash/nand/fsl_nand_flash.h"

extern "C" nand_handle_t *BOARD_GetNANDHandle(void);

#define DATA_IN (0)
#define DATA_OUT (1)
#define USB_DEVICE_MSC_WRITE_BUFF_SIZE (512 * 32U)
#define USB_DEVICE_MSC_READ_BUFF_SIZE (512 * 32U)
#define LOGICAL_UNIT_SUPPORTED (1)

namespace coralmicro {
namespace {
constexpr int kPagesPerBlock = 64;
constexpr int kFilesystemBaseBlock = 12;
constexpr size_t kPageSize = 2048;
constexpr size_t kFlashSize = 0x10000 * 1024;
constexpr uint32_t kInvalidDataPattern = __htonl(0xdeadbeef);
}  // namespace

uint32_t g_mscReadRequestBuffer[kPageSize];
uint32_t g_mscWriteRequestBuffer[kPageSize];

usb_device_inquiry_data_fromat_struct_t g_InquiryInfo = {
    (USB_DEVICE_MSC_UFI_PERIPHERAL_QUALIFIER
     << USB_DEVICE_MSC_UFI_PERIPHERAL_QUALIFIER_SHIFT) |
        USB_DEVICE_MSC_UFI_PERIPHERAL_DEVICE_TYPE,
    (uint8_t)(USB_DEVICE_MSC_UFI_REMOVABLE_MEDIUM_BIT
              << USB_DEVICE_MSC_UFI_REMOVABLE_MEDIUM_BIT_SHIFT),
    USB_DEVICE_MSC_UFI_VERSIONS,
    0x02,
    USB_DEVICE_MSC_UFI_ADDITIONAL_LENGTH,
    {0x00, 0x00, 0x00},
    {'C', 'O', 'R', 'A', 'L'},
    {'M', 'A', 'S', 'S', ' ', 'S', 'T', 'O', 'R', 'A', 'G', 'E'},
    {'0', '0', '0', '1'}};
usb_device_mode_parameters_header_struct_t g_ModeParametersHeader = {
    /*refer to ufi spec mode parameter header*/
    0x0000, /*!< Mode Data Length*/
    0x00,   /*!<Default medium type (current mounted medium type)*/
    0x00,   /*!MODE SENSE command, a Write Protected bit of zero indicates the
               medium is write enabled*/
    {0x00, 0x00, 0x00, 0x00} /*!<This bit should be set to zero*/
};

std::map<class_handle_t, MscUms *> MscUms::handle_map_;

MscUms::MscUms() {}

void MscUms::Init(uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t data_iface) {
  bulk_in_ep_ = bulk_in_ep;
  bulk_out_ep_ = bulk_out_ep;
  msc_ums_data_endpoints_[DATA_IN].endpointAddress = bulk_in_ep | (USB_IN << 7);
  msc_ums_data_endpoints_[DATA_OUT].endpointAddress =
      bulk_out_ep | (USB_OUT << 7);
  msc_ums_interfaces_[0].interfaceNumber = data_iface;
}

void MscUms::SetClassHandle(class_handle_t class_handle) {
  handle_map_[class_handle] = this;
  class_handle_ = class_handle;
}

bool MscUms::HandleEvent(uint32_t event, void *param) {
  usb_status_t status = kStatus_USB_Success;

  switch (event) {
    case kUSB_DeviceEventSetConfiguration:
      // Don't care.
      break;
    default:
      printf("%s unhandled event %ld\r\n", __PRETTY_FUNCTION__, event);
      status = kStatus_USB_Error;
  }
  return (status == kStatus_USB_Success);
}

usb_status_t MscUms::Handler(uint32_t event, void *param) {
  usb_status_t error = kStatus_USB_Success;
  status_t errorCode = kStatus_Success;
  nand_handle_t *nand = BOARD_GetNANDHandle();
  usb_device_lba_information_struct_t *lbaInformation;
  usb_device_lba_app_struct_t *lba;
  usb_device_ufi_app_struct_t *ufi;
  usb_device_capacity_information_struct_t *capacityInformation;
  CHECK(nand);

  switch (event) {
    case kUSB_DeviceMscEventReadResponse:
      lba = (usb_device_lba_app_struct_t *)param;
      break;
    case kUSB_DeviceMscEventWriteResponse:
      lba = (usb_device_lba_app_struct_t *)param;
      if (lba->offset == 0 && std::memcmp(lba->buffer, &kInvalidDataPattern,
                                          sizeof(kInvalidDataPattern)) == 0) {
        uint32_t block = __ntohl(*reinterpret_cast<uint32_t *>(
            lba->buffer + sizeof(kInvalidDataPattern)));
        uint32_t erase_block = kFilesystemBaseBlock + block;
        errorCode = Nand_Flash_Erase_Block(nand, erase_block);
        if (errorCode != kStatus_Success) {
          printf("Nand_Flash_Erase_Block(%lu) failed (%ld), block %lu\r\n",
                 erase_block, errorCode, block);
        }
      } else {
        size_t size = lba->size;
        uint8_t *buf = lba->buffer;
        uint32_t page_index = lba->offset;
        while (size != 0) {
          auto write_size = std::min(kPageSize, size);
          auto write_index = kFilesystemBaseBlock * kPagesPerBlock + page_index;
          DCACHE_CleanInvalidateByRange(reinterpret_cast<uint32_t>(buf),
                                        write_size);
          errorCode =
              Nand_Flash_Page_Program(nand, write_index, buf, write_size);
          if (errorCode != kStatus_Success) {
            printf(
                "Nand_Flash_Page_Program(%lu, %u) failed (%ld), page %lu\r\n",
                write_index, write_size, errorCode, page_index);
            break;
          }
          ++page_index;
          buf += write_size;
          size -= write_size;
        }
      }
      if (errorCode != kStatus_Success) {
        error = kStatus_USB_InvalidRequest;
      }
      break;
    case kUSB_DeviceMscEventWriteRequest:
      lba = (usb_device_lba_app_struct_t *)param;
      /*get a buffer to store the data from host*/
      lba->buffer = (uint8_t *)&g_mscWriteRequestBuffer[0];
      break;
    case kUSB_DeviceMscEventReadRequest:
      lba = (usb_device_lba_app_struct_t *)param;
      lba->buffer = (uint8_t *)&g_mscReadRequestBuffer[0];
      {
        size_t size = lba->size;
        uint8_t *buf = lba->buffer;
        uint32_t page_index = lba->offset;
        while (size != 0) {
          auto read_size = std::min(kPageSize, size);
          auto read_index = kFilesystemBaseBlock * kPagesPerBlock + page_index;
          errorCode = Nand_Flash_Read_Page(nand, read_index, buf, read_size);
          if (errorCode != kStatus_Success) {
            printf("Nand_Flash_Read_Page(%lu, %u) failed (%ld), page %lu\r\n",
                   read_index, read_size, errorCode, page_index);
            break;
          }
          ++page_index;
          buf += read_size;
          size -= read_size;
        }
        DCACHE_InvalidateByRange(reinterpret_cast<uint32_t>(lba->buffer),
                                 lba->size);
      }
      if (errorCode != kStatus_Success) {
        for (size_t i = 0;
             i < sizeof(g_mscReadRequestBuffer) / sizeof(kInvalidDataPattern);
             ++i) {
          memcpy(&g_mscReadRequestBuffer[i * sizeof(kInvalidDataPattern)],
                 &kInvalidDataPattern, sizeof(kInvalidDataPattern));
        }
        error = kStatus_USB_InvalidRequest;
      }
      break;
    case kUSB_DeviceMscEventGetLbaInformation:
      lbaInformation = (usb_device_lba_information_struct_t *)param;
      lbaInformation->logicalUnitNumberSupported = LOGICAL_UNIT_SUPPORTED;
      lbaInformation->logicalUnitInformations[0].lengthOfEachLba = kPageSize;
      lbaInformation->logicalUnitInformations[0].totalLbaNumberSupports =
          kFlashSize / kPageSize - kFilesystemBaseBlock * kPagesPerBlock;
      lbaInformation->logicalUnitInformations[0].bulkInBufferSize =
          sizeof(g_mscReadRequestBuffer);
      lbaInformation->logicalUnitInformations[0].bulkOutBufferSize =
          sizeof(g_mscWriteRequestBuffer);
      break;
    case kUSB_DeviceMscEventTestUnitReady:
      /*change the test unit ready command's sense data if need, be careful to
       * modify*/
      ufi = (usb_device_ufi_app_struct_t *)param;
      break;
    case kUSB_DeviceMscEventInquiry:
      ufi = (usb_device_ufi_app_struct_t *)param;
      ufi->size = sizeof(usb_device_inquiry_data_fromat_struct_t);
      ufi->buffer = (uint8_t *)&g_InquiryInfo;
      break;
    case kUSB_DeviceMscEventModeSense:
      ufi = (usb_device_ufi_app_struct_t *)param;
      ufi->size = sizeof(usb_device_mode_parameters_header_struct_t);
      ufi->buffer = (uint8_t *)&g_ModeParametersHeader;
      break;
    case kUSB_DeviceMscEventModeSelectResponse:
      ufi = (usb_device_ufi_app_struct_t *)param;
      break;
    case kUSB_DeviceMscEventModeSelect:
    case kUSB_DeviceMscEventFormatComplete:
    case kUSB_DeviceMscEventRemovalRequest:
    case kUSB_DeviceMscEventRequestSense:
      error = kStatus_USB_InvalidRequest;
      break;
    case kUSB_DeviceMscEventReadCapacity:
      capacityInformation = (usb_device_capacity_information_struct_t *)param;
      capacityInformation->lengthOfEachLba = kPageSize;
      capacityInformation->totalLbaNumberSupports =
          kFlashSize / kPageSize - kFilesystemBaseBlock * kPagesPerBlock;
      break;
    case kUSB_DeviceMscEventReadFormatCapacity:
      capacityInformation = (usb_device_capacity_information_struct_t *)param;
      capacityInformation->lengthOfEachLba = kPageSize;
      capacityInformation->totalLbaNumberSupports =
          kFlashSize / kPageSize - kFilesystemBaseBlock * kPagesPerBlock;
      break;
    default:
      error = kStatus_USB_InvalidRequest;
      break;
  }
  return error;
}

usb_status_t MscUms::Handler(class_handle_t class_handle, uint32_t event,
                             void *param) {
  return handle_map_[class_handle]->Handler(event, param);
}

}  // namespace coralmicro
