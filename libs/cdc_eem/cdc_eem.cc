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

#include "libs/cdc_eem/cdc_eem.h"

#include "libs/base/check.h"
#include "libs/base/tasks.h"
#include "libs/base/utils.h"
#include "libs/nxp/rt1176-sdk/usb_device_cdc_eem.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/etharp.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/netif/ethernet.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.h"

extern "C" void start_dhcp_server(uint32_t local_addr);

#include <cstring>
#include <memory>
#include <vector>

#define DATA_OUT (1)
#define DATA_IN (0)

namespace coralmicro {
std::map<class_handle_t, CdcEem *> CdcEem::handle_map_;

void CdcEem::Init(uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t data_iface) {
  bulk_in_ep_ = bulk_in_ep;
  bulk_out_ep_ = bulk_out_ep;
  cdc_eem_data_endpoints_[DATA_IN].endpointAddress = bulk_in_ep | (USB_IN << 7);
  cdc_eem_data_endpoints_[DATA_OUT].endpointAddress =
      bulk_out_ep | (USB_OUT << 7);
  cdc_eem_interfaces_[0].interfaceNumber = data_iface;
  tx_queue_ = xQueueCreate(10, sizeof(void *));
  CHECK(tx_queue_);
  CHECK(xTaskCreate(CdcEem::StaticTaskFunction, "cdc_eem_task",
                    configMINIMAL_STACK_SIZE * 10, this, kUsbDeviceTaskPriority,
                    nullptr) == pdPASS);

  std::string usb_ip;
  if (!GetUsbIpAddress(&usb_ip) ||
      !ipaddr_aton(usb_ip.c_str(), &netif_ipaddr_)) {
    IP4_ADDR(&netif_ipaddr_, 10, 10, 10, 1);
  }
  IP4_ADDR(&netif_netmask_, 255, 255, 255, 0);
  IP4_ADDR(&netif_gw_, 0, 0, 0, 0);
  netifapi_netif_add(&netif_, &netif_ipaddr_, &netif_netmask_, &netif_gw_, this,
                     CdcEem::StaticNetifInit, tcpip_input);
  netifapi_netif_set_default(&netif_);
  netifapi_netif_set_link_up(&netif_);
  netifapi_netif_set_up(&netif_);
  start_dhcp_server(netif_ipaddr_.addr);
}

void CdcEem::TaskFunction(void *param) {
  while (true) {
    std::vector<uint8_t> *packet;
    if (xQueueReceive(tx_queue_, &packet, portMAX_DELAY) == pdTRUE) {
      TransmitFrame(packet->data(), packet->size());
      delete packet;
    }
  }
}

err_t CdcEem::NetifInit(struct netif *netif) {
  netif->name[0] = 'u';
  netif->name[1] = 's';
  netif->output = etharp_output;
  netif->linkoutput = CdcEem::StaticTxFunc;
  netif->mtu = 300;
  netif->hwaddr_len = 6;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

  netif->hwaddr[0] = 0x00;
  netif->hwaddr[1] = 0x1A;
  netif->hwaddr[2] = 0x11;
  netif->hwaddr[3] = 0xBA;
  netif->hwaddr[4] = 0xDF;
  netif->hwaddr[5] = 0xAD;

  return ERR_OK;
}

err_t CdcEem::TxFunc(struct netif *netif, struct pbuf *p) {
  auto *packet = new std::vector<uint8_t>(p->tot_len);
  if (pbuf_copy_partial(p, packet->data(), p->tot_len, 0) != p->tot_len) {
    delete packet;
    return ERR_IF;
  }

  if (xQueueSendToBack(tx_queue_, &packet, 0) != pdTRUE) {
    delete packet;
    return ERR_IF;
  }

  return ERR_OK;
}

err_t CdcEem::TransmitFrame(void *buffer, uint32_t length) {
  if (endianness_ == Endianness::kUnknown) {
    return ERR_IF;
  }
  usb_status_t status;
  uint32_t crc = PP_HTONL(0xdeadbeef);
  uint16_t *header = reinterpret_cast<uint16_t *>(tx_buffer_);
  *header = (0 << EEM_DATA_CRC_SHIFT) |
            ((length + sizeof(uint32_t)) & EEM_DATA_LEN_MASK);
  if (endianness_ == Endianness::kBigEndian) {
    *header = htons(*header);
  }
  std::memcpy(tx_buffer_ + sizeof(uint16_t), buffer, length);
  std::memcpy(tx_buffer_ + sizeof(uint16_t) + length, &crc, sizeof(uint32_t));
  while (true) {
    status = USB_DeviceCdcEemSend(class_handle_, bulk_in_ep_, tx_buffer_,
                                  sizeof(uint16_t) + length + sizeof(uint32_t));
    if (status == kStatus_USB_Busy) {
      taskYIELD();
    } else {
      break;
    }
  }
  if (status != kStatus_USB_Success) {
    DbgConsole_Printf("[EEM] USB_DeviceCdcEemSend failed, ERR_IF\r\n");
    return ERR_IF;
  }

  return (status == kStatus_USB_Success) ? ERR_OK : ERR_IF;
}

err_t CdcEem::ReceiveFrame(uint8_t *buffer, uint32_t length) {
  struct pbuf *frame = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
  if (!frame) {
    printf("Failed to allocate pbuf\r\n");
    return ERR_BUF;
  }
  pbuf_take(frame, buffer, length);
  err_t ret = netif_.input(frame, &netif_);
  if (ret != ERR_OK) {
    printf("tcpip_input() failed %d\r\n", ret);
    pbuf_free_callback(frame);
    return ERR_IF;
  }

  return ERR_OK;
}

usb_status_t CdcEem::SetControlLineState(
    usb_device_cdc_eem_request_param_struct_t *eem_param) {
  uint8_t dte_status = eem_param->setupValue;
  uint16_t uart_state = 0;

  uint8_t carrier_present =
      (dte_status & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION);
  if (carrier_present) {
    uart_state |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
  } else {
    uart_state &= ~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
  }

  uint8_t dte_present =
      (dte_status & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE);
  if (dte_present) {
    uart_state |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
  } else {
    uart_state &= ~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
  }

  uint8_t serial_state_buffer[10] = {
      0xA1,  // NotifyRequestType
      USB_DEVICE_CDC_NOTIF_SERIAL_STATE,
      0x00,
      0x00,
      static_cast<uint8_t>(eem_param->interfaceIndex),
      0x00,
      0x02,  // UartBitmapSize
      0x00,
      static_cast<uint8_t>(uart_state & 0xFF),
      static_cast<uint8_t>((uart_state >> 8) & 0xFF),
  };

  auto *cdc_eem =
      reinterpret_cast<usb_device_cdc_eem_struct_t *>(class_handle_);
  usb_status_t ret = kStatus_USB_Error;
  if (cdc_eem->hasSentState == 0) {
    ret = USB_DeviceCdcEemSend(class_handle_, bulk_in_ep_, serial_state_buffer,
                               sizeof(serial_state_buffer));
    if (ret != kStatus_USB_Success) {
      DbgConsole_Printf("USB_DeviceCdcEemSend failed in %s\r\n", __func__);
    }
    cdc_eem->hasSentState = 1;
  }

  return ret;
}

void CdcEem::DetectEndianness(uint32_t packet_length) {
  if (endianness_ == Endianness::kUnknown) {
    uint16_t packet_hdr_le = *reinterpret_cast<uint16_t *>(rx_buffer_);
    uint16_t packet_hdr_be = ntohs(*reinterpret_cast<uint16_t *>(rx_buffer_));
    // Two-byte packets are usually EEM command packets, but we can't
    // detect endianness from them with certainty -- so we will not try.
    if (packet_length <= sizeof(uint16_t)) {
      return;
    }
    uint16_t le_len =
        ((packet_hdr_le & EEM_DATA_LEN_MASK) >> EEM_DATA_LEN_SHIFT);
    uint16_t be_len =
        ((packet_hdr_be & EEM_DATA_LEN_MASK) >> EEM_DATA_LEN_SHIFT);
    if (le_len == (packet_length - sizeof(uint16_t))) {
      endianness_ = Endianness::kLittleEndian;
    } else if (be_len == (packet_length - sizeof(uint16_t))) {
      endianness_ = Endianness::kBigEndian;
    } else {
      DbgConsole_Printf("[EEM] Unable to detect endianness\r\n");
      return;
    }
  }
}

void CdcEem::ProcessPacket(uint32_t packet_length) {
  DetectEndianness(packet_length);
  if (endianness_ == Endianness::kUnknown) {
    return;
  }

  uint16_t packet_hdr = *reinterpret_cast<uint16_t *>(rx_buffer_);
  if (endianness_ == Endianness::kBigEndian) {
    packet_hdr = ntohs(packet_hdr);
  }

  if (packet_hdr & EEM_HEADER_TYPE_MASK) {
    uint16_t opcode =
        (packet_hdr & EEM_COMMAND_OPCODE_MASK) >> EEM_COMMAND_OPCODE_SHIFT;
    uint16_t param =
        (packet_hdr & EEM_COMMAND_PARAM_MASK) >> EEM_COMMAND_PARAM_SHIFT;
    uint8_t *data = rx_buffer_ + sizeof(uint16_t);
    (void)param;
    (void)data;
    switch (opcode) {
      case EEM_COMMAND_ECHO_RESPONSE:
        break;
      default:
        DbgConsole_Printf("Unhandled EEM opcode: %u\r\n", opcode);
    }
  } else {
    uint16_t checksum = (packet_hdr & EEM_DATA_CRC_MASK) >> EEM_DATA_CRC_SHIFT;
    uint16_t len = (packet_hdr & EEM_DATA_LEN_MASK) >> EEM_DATA_LEN_SHIFT;
    if (len == 0) {
      return;
    }
    uint8_t *data = rx_buffer_ + sizeof(uint16_t);
    uint16_t data_len = len - sizeof(uint32_t);
    // TODO(atv): We should validate checksum. But we won't (for now). See if
    // the stack handles that?
    (void)checksum;
    ReceiveFrame(data, data_len);
  }
}

bool CdcEem::HandleEvent(uint32_t event, void *param) {
  usb_status_t status;
  switch (event) {
    case kUSB_DeviceEventSetConfiguration:
      break;
    case kUSB_DeviceEventSetInterface:
      USB_DeviceCdcEemRecv(class_handle_, bulk_out_ep_, rx_buffer_,
                           cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize);
      break;
    default:
      DbgConsole_Printf("%s unhandled event %d\r\n", __PRETTY_FUNCTION__,
                        event);
      return false;
  }
  return (status == kStatus_USB_Success);
}

usb_status_t CdcEem::Handler(uint32_t event, void *param) {
  usb_status_t ret = kStatus_USB_Error;
  auto *ep_cb =
      static_cast<usb_device_endpoint_callback_message_struct_t *>(param);
  auto *eem_param =
      static_cast<usb_device_cdc_eem_request_param_struct_t *>(param);

  switch (event) {
    case kUSB_DeviceEemEventRecvResponse: {
      ProcessPacket(ep_cb->length);
      ret =
          USB_DeviceCdcEemRecv(class_handle_, bulk_out_ep_, rx_buffer_,
                               cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize);
      break;
    }
    case kUSB_DeviceEemEventSendResponse:
      if (ep_cb->length != 0 &&
          (ep_cb->length % cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize) ==
              0) {
        ret = USB_DeviceCdcEemSend(class_handle_, bulk_in_ep_, nullptr, 0);
      } else {
        if (ep_cb->buffer || (!ep_cb->buffer && ep_cb->length == 0)) {
          ret = USB_DeviceCdcEemRecv(
              class_handle_, bulk_out_ep_, rx_buffer_,
              cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize);
        }
      }
      break;
    case kUSB_DeviceCdcEventSetControlLineState:
      ret = SetControlLineState(eem_param);
      break;
    default:
      DbgConsole_Printf("Unhandled EEM event: %d\r\n", event);
  }

  return ret;
}

}  // namespace coralmicro
