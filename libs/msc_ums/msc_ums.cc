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

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/components/flash/nand/fsl_nand_flash.h"

extern "C" nand_handle_t *BOARD_GetNANDHandle(void);

#define DATA_IN (0)
#define DATA_OUT (1)
#define USB_MSC_INTERFACE_COUNT (1U)
#define USB_MSC_INTERFACE_ALTERNATE_COUNT (1U)
#define USB_MSC_CONFIGURE_INDEX (1U)
#define USB_DEVICE_MSC_WRITE_BUFF_SIZE (512 * 32U)
#define USB_DEVICE_MSC_READ_BUFF_SIZE (512 * 32U)
#define LOGICAL_UNIT_SUPPORTED (1)

namespace coralmicro {
namespace {
constexpr int kPagesPerBlock = 64;
constexpr int kMaxPagesPerTransfer = 16;
constexpr int kFilesystemBaseBlock = 12;
constexpr size_t kPageSize = 2048;
}  // namespace

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint32_t g_mscReadRequestBuffer[(USB_DEVICE_MSC_READ_BUFF_SIZE * 2) >> 2];

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint32_t g_mscWriteRequestBuffer[(USB_DEVICE_MSC_WRITE_BUFF_SIZE * 2) >> 2];
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
typedef struct _usb_msc_struct {
  usb_device_handle deviceHandle;
  class_handle_t mscHandle;
  TaskHandle_t device_task_handle;
  TaskHandle_t application_task_handle;
  uint8_t diskLock;
  uint8_t read_write_error;
  uint8_t currentConfiguration;
  uint8_t currentInterfaceAlternateSetting[USB_MSC_INTERFACE_COUNT];
  uint8_t speed;
  uint8_t attach;
} usb_msc_struct_t;

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
  usb_status_t status;
  uint16_t *temp16 = (uint16_t *)param;
  uint8_t *temp8 = (uint8_t *)param;
  usb_msc_struct_t *msc_handle = (usb_msc_struct_t *)class_handle_;

  switch (event) {
    case kUSB_DeviceEventSetConfiguration:
      if (0U == (*temp8)) {
        msc_handle->attach = 0;
        msc_handle->currentConfiguration = 0;
        status = kStatus_USB_Success;
      } else if (USB_MSC_CONFIGURE_INDEX == (*temp8)) {
        msc_handle->attach = 1;
        msc_handle->currentConfiguration = *temp8;
        status = kStatus_USB_Success;
      }
      break;
    case kUSB_DeviceEventSetInterface:
      initialized_ = true;
      if (msc_handle->attach) {
        uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
        uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
        if (interface < USB_MSC_INTERFACE_COUNT) {
          if (alternateSetting < USB_MSC_INTERFACE_ALTERNATE_COUNT) {
            msc_handle->currentInterfaceAlternateSetting[interface] =
                alternateSetting;
            status = kStatus_USB_Success;
          }
        }
      }
      break;
    case kUSB_DeviceEventGetConfiguration:
      break;
    default:
      DbgConsole_Printf("%s unhandled event %d\r\n", __PRETTY_FUNCTION__,
                        event);
      return false;
  }
  return (status == kStatus_USB_Success);
}

usb_status_t MscUms::Handler(uint32_t event, void *param) {
  usb_status_t error = kStatus_USB_Success;
  status_t errorCode = kStatus_Success;
  usb_device_lba_information_struct_t *lbaInformation;
  usb_device_lba_app_struct_t *lba;
  usb_device_ufi_app_struct_t *ufi;
  usb_device_capacity_information_struct_t *capacityInformation;
  usb_msc_struct_t *msc_handle = (usb_msc_struct_t *)class_handle_;

  switch (event) {
    case kUSB_DeviceMscEventReadResponse:
      lba = (usb_device_lba_app_struct_t *)param;
      break;
    case kUSB_DeviceMscEventWriteResponse:
      lba = (usb_device_lba_app_struct_t *)param;
      /*write the data to sd card*/
      if (0 != lba->size) {
        errorCode = kStatus_Success;
        nand_handle_t *nand = BOARD_GetNANDHandle();
        if (nand) {
          size_t size = lba->size;
          uint8_t *buf = lba->buffer;
          uint32_t page_index = lba->offset;
          while (size != 0) {
            auto write_size = std::min(kPageSize, size);
            status_t errorCode = Nand_Flash_Page_Program(
                nand, kFilesystemBaseBlock * kPagesPerBlock + page_index, buf,
                write_size);
            if (errorCode != kStatus_Success) {
              break;
            }
            ++page_index;
            buf += write_size;
            size -= write_size;
          }
        } else {
          errorCode = kStatus_Fail;
        }
        if (kStatus_Success != errorCode) {
          msc_handle->read_write_error = 1;
          DbgConsole_Printf(
              "Write error, error = 0xx%x \t Please check write request buffer "
              "size(must be less than 128 "
              "sectors)\r\n",
              error);
          error = kStatus_USB_Error;
        }
      } else {
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

      /*read the data from sd card, then store these data to the read buffer*/
      errorCode = kStatus_Success;
      {
        nand_handle_t *nand = BOARD_GetNANDHandle();
        if (nand) {
          size_t size = lba->size;
          uint8_t *buf = lba->buffer;
          uint32_t page_index = lba->offset;
          while (size != 0) {
            auto read_size = std::min(kPageSize, size);
            status_t errorCode = Nand_Flash_Read_Page(
                nand, kFilesystemBaseBlock * kPagesPerBlock + page_index, buf,
                read_size);
            if (errorCode != kStatus_Success) {
              break;
            }
            ++page_index;
            buf += read_size;
            size -= read_size;
          }
        } else {
          errorCode = kStatus_Fail;
        }
      }
      if (kStatus_Success != errorCode) {
        msc_handle->read_write_error = 1;
        DbgConsole_Printf(
            "Read error, error = 0xx%x \t Please check read request buffer "
            "size(must be less than 128 "
            "sectors)\r\n",
            error);
        error = kStatus_USB_Error;
      }
      break;
    case kUSB_DeviceMscEventGetLbaInformation:
      lbaInformation = (usb_device_lba_information_struct_t *)param;
      lbaInformation->logicalUnitNumberSupported = LOGICAL_UNIT_SUPPORTED;
      lbaInformation->logicalUnitInformations[0].lengthOfEachLba = kPageSize;
      lbaInformation->logicalUnitInformations[0].totalLbaNumberSupports =
          kMaxPagesPerTransfer * kPageSize;
      lbaInformation->logicalUnitInformations[0].bulkInBufferSize =
          USB_DEVICE_MSC_READ_BUFF_SIZE;
      lbaInformation->logicalUnitInformations[0].bulkOutBufferSize =
          USB_DEVICE_MSC_WRITE_BUFF_SIZE;
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
          kMaxPagesPerTransfer * kPageSize;
      break;
    case kUSB_DeviceMscEventReadFormatCapacity:
      capacityInformation = (usb_device_capacity_information_struct_t *)param;
      capacityInformation->lengthOfEachLba = kPageSize;
      capacityInformation->totalLbaNumberSupports =
          kMaxPagesPerTransfer * kPageSize;
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
