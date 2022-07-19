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

#include "libs/cdc_acm/cdc_acm.h"

#include <cstdio>
#include <cstring>

#include "libs/base/check.h"
#include "libs/base/reset.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.h"

#define DATA_OUT (1)
#define DATA_IN (0)

namespace coralmicro {
std::map<class_handle_t, CdcAcm*> CdcAcm::handle_map_;

void CdcAcm::Init(uint8_t interrupt_in_ep, uint8_t bulk_in_ep,
                  uint8_t bulk_out_ep, uint8_t comm_iface, uint8_t data_iface,
                  RxHandler rx_handler) {
  interrupt_in_ep_ = interrupt_in_ep;
  bulk_in_ep_ = bulk_in_ep;
  bulk_out_ep_ = bulk_out_ep;
  rx_handler_ = rx_handler;
  cdc_acm_comm_endpoints_[0].endpointAddress = interrupt_in_ep | (USB_IN << 7);
  cdc_acm_data_endpoints_[DATA_IN].endpointAddress = bulk_in_ep | (USB_IN << 7);
  cdc_acm_data_endpoints_[DATA_OUT].endpointAddress =
      bulk_out_ep | (USB_OUT << 7);
  cdc_acm_interfaces_[0].interfaceNumber = comm_iface;
  cdc_acm_interfaces_[1].interfaceNumber = data_iface;
  tx_semaphore_ = xSemaphoreCreateBinary();
  CHECK(tx_semaphore_);
}

usb_status_t CdcAcm::SetControlLineState(
    usb_device_cdc_acm_request_param_struct_t* acm_param) {
  usb_status_t ret = kStatus_USB_Error;

  uint8_t dte_status = acm_param->setupValue;
  uint16_t uart_state = 0;

  uint8_t dte_present =
      (dte_status & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE);
  uint8_t carrier_present =
      (dte_status & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION);
  if (carrier_present) {
    uart_state |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
  } else {
    uart_state &= ~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
  }

  if (dte_present) {
    uart_state |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
  } else {
    uart_state &= ~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
  }

  serial_state_buffer_[0] = 0xA1;  // NotifyRequestType
  serial_state_buffer_[1] = USB_DEVICE_CDC_NOTIF_SERIAL_STATE;
  serial_state_buffer_[2] = 0x00;
  serial_state_buffer_[3] = 0x00;
  serial_state_buffer_[4] = acm_param->interfaceIndex;
  serial_state_buffer_[5] = 0x00;
  serial_state_buffer_[6] = 0x02;  // UartBitmapSize
  serial_state_buffer_[7] = 0x00;

  uint8_t* uart_bitmap = &serial_state_buffer_[8];
  uart_bitmap[0] = uart_state & 0xFF;
  uart_bitmap[1] = (uart_state >> 8) & 0xFF;

  uint32_t len = sizeof(serial_state_buffer_);
  auto* cdc_acm = reinterpret_cast<usb_device_cdc_acm_struct_t*>(class_handle_);
  if (cdc_acm->hasSentState == 0) {
    ret = USB_DeviceCdcAcmSend(class_handle_, interrupt_in_ep_,
                               serial_state_buffer_, len);
    if (ret != kStatus_USB_Success) {
      DbgConsole_Printf("USB_DeviceCdcAcmSend failed in %s\r\n", __func__);
    }
    cdc_acm->hasSentState = 1;
  }

  can_transmit_ =
      (dte_status & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) > 0;

  if (line_coding_.dwDTERate == 1200) {
    ResetToBootloader();
  }

  return ret;
}

void CdcAcm::SetConfiguration() {
  usb_status_t status =
      USB_DeviceCdcAcmRecv(class_handle_, bulk_out_ep_, rx_buffer_,
                           cdc_acm_data_endpoints_[DATA_OUT].maxPacketSize);
  if (status != kStatus_USB_Success) {
    DbgConsole_Printf("USB_DeviceCdcAcmRecv failed in %s: %d\r\n",
                      __PRETTY_FUNCTION__, status);
  }
}

bool CdcAcm::Transmit(const uint8_t* buffer, const size_t length) {
  if (!can_transmit_) {
    return false;
  }

  usb_status_t status;
  if (length > 512) {
    DbgConsole_Printf("%s data larger than tx_buffer_\r\n",
                      __PRETTY_FUNCTION__);
    return false;
  }

  std::memcpy(tx_buffer_, buffer, length);
  status = USB_DeviceCdcAcmSend(class_handle_, bulk_in_ep_, tx_buffer_, length);

  if (status != kStatus_USB_Success) {
    return false;
  }
  if (xSemaphoreTake(tx_semaphore_, pdMS_TO_TICKS(200)) == pdTRUE) {
    return true;
  } else {
    return false;
  }
}

bool CdcAcm::HandleEvent(uint32_t event, void* param) {
  switch (event) {
    case kUSB_DeviceEventSetConfiguration:
      SetConfiguration();
      break;
    case kUSB_DeviceEventSetInterface:
      break;
    default:
      DbgConsole_Printf("%s: unhandled event %lu\r\n", __PRETTY_FUNCTION__,
                        event);
      return false;
  }
  return true;
}

usb_status_t CdcAcm::Handler(uint32_t event, void* param) {
  usb_status_t ret = kStatus_USB_Error;
  auto* ep_cb =
      static_cast<usb_device_endpoint_callback_message_struct_t*>(param);
  auto* acm_param =
      static_cast<usb_device_cdc_acm_request_param_struct_t*>(param);
  switch (event) {
    case kUSB_DeviceCdcEventSendResponse:
      if ((ep_cb->length != 0) &&
          (ep_cb->length % cdc_acm_data_endpoints_[DATA_OUT].maxPacketSize) ==
              0) {
        // The packet is equal to the size of the endpoint. Send a zero length
        // packet to let the other side know we're done.
        ret = USB_DeviceCdcAcmSend(class_handle_, bulk_in_ep_, nullptr, 0);
      } else {
        if (ep_cb->buffer || (!ep_cb->buffer && ep_cb->length == 0)) {
          CHECK(xSemaphoreGive(tx_semaphore_) == pdTRUE);
          ret = USB_DeviceCdcAcmRecv(
              class_handle_, bulk_out_ep_, rx_buffer_,
              cdc_acm_data_endpoints_[DATA_OUT].maxPacketSize);
        }
      }
      break;
    case kUSB_DeviceCdcEventRecvResponse:
      if (rx_handler_) {
        rx_handler_(rx_buffer_, ep_cb->length);
      }
      ret =
          USB_DeviceCdcAcmRecv(class_handle_, bulk_out_ep_, rx_buffer_,
                               cdc_acm_data_endpoints_[DATA_OUT].maxPacketSize);
      break;
    case kUSB_DeviceCdcEventSerialStateNotif:
      reinterpret_cast<usb_device_cdc_acm_struct_t*>(class_handle_)
          ->hasSentState = 0;
      ret = kStatus_USB_Success;
      break;
    case kUSB_DeviceCdcEventSetLineCoding:
      if (*(acm_param->length) == sizeof(line_coding_)) {
        memcpy(&line_coding_, *(acm_param->buffer), sizeof(line_coding_));
      }
      ret = kStatus_USB_Success;
      break;
    case kUSB_DeviceCdcEventSetControlLineState:
      ret = SetControlLineState(acm_param);
      break;
    default:
      DbgConsole_Printf("%s: unhandled event %lu\r\n", __PRETTY_FUNCTION__,
                        event);
  }
  return ret;
}

}  // namespace coralmicro
