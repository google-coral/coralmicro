#include "libs/CdcEem/cdc_eem.h"

#include "libs/base/utils.h"
#include "libs/nxp/rt1176-sdk/usb_device_cdc_eem.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/netif/ethernet.h"

extern "C" {
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/app/dhcp_server.h"
}

#include <memory>

#define DATA_OUT (1)
#define DATA_IN  (0)

namespace valiant {

std::map<class_handle_t, CdcEem*> CdcEem::handle_map_;

CdcEem::CdcEem() {}

void CdcEem::Init(uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t data_iface) {
    bulk_in_ep_ = bulk_in_ep;
    bulk_out_ep_ = bulk_out_ep;
    cdc_eem_data_endpoints_[DATA_IN].endpointAddress = bulk_in_ep | (USB_IN << 7);
    cdc_eem_data_endpoints_[DATA_OUT].endpointAddress = bulk_out_ep | (USB_OUT << 7);
    cdc_eem_interfaces_[0].interfaceNumber = data_iface;
    tx_semaphore_ = xSemaphoreCreateBinary();

    if (!utils::GetUSBIPAddress(&netif_ipaddr_)) {
        IP4_ADDR(&netif_ipaddr_, 10, 10, 10, 1);
    }
    IP4_ADDR(&netif_netmask_, 255, 255, 255, 0);
    IP4_ADDR(&netif_gw_, 0, 0, 0, 0);
    netifapi_netif_add(&netif_, &netif_ipaddr_, &netif_netmask_, &netif_gw_, this, CdcEem::StaticNetifInit, ethernet_input);
    netifapi_netif_set_default(&netif_);
    netifapi_netif_set_link_up(&netif_);
    netifapi_netif_set_up(&netif_);
    start_dhcp_server(netif_ipaddr_.addr);
}

err_t CdcEem::StaticNetifInit(struct netif *netif) {
    CdcEem *instance = reinterpret_cast<CdcEem*>(netif->state);
    return instance->NetifInit(netif);
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

err_t CdcEem::StaticTxFunc(struct netif *netif, struct pbuf *p) {
    CdcEem *instance = reinterpret_cast<CdcEem*>(netif->state);
    return instance->TxFunc(netif, p);
}

err_t CdcEem::TxFunc(struct netif *netif, struct pbuf *p) {
    std::unique_ptr<uint8_t> combined_pbuf(nullptr);
    uint8_t *tx_ptr = nullptr;
    uint32_t tx_len = p->tot_len;
    if ((p->next == NULL) && (p->tot_len == p->len)) {
        tx_ptr = reinterpret_cast<uint8_t*>(p->payload);
    } else {
        combined_pbuf.reset(reinterpret_cast<uint8_t*>(malloc(p->tot_len)));
        tx_ptr = combined_pbuf.get();
        struct pbuf *tmp_p = p;
        int offset = 0;
        do {
            memcpy(tx_ptr + offset, reinterpret_cast<uint8_t*>(tmp_p->payload), tmp_p->len);
            offset += tmp_p->len;
            tmp_p = tmp_p->next;
            if (!tmp_p)
                break;
        } while (true);
    }

    return TransmitFrame(tx_ptr, tx_len);
}

void CdcEem::SetClassHandle(class_handle_t class_handle) {
    handle_map_[class_handle] = this;
    class_handle_ = class_handle;
}

err_t CdcEem::TransmitFrame(uint8_t* buffer, uint32_t length) {
    usb_status_t status;
    uint32_t crc = PP_HTONL(0xdeadbeef);
    uint16_t *header = (uint16_t*)tx_buffer_;
    // printf("TransmitFrame %p\r\n", length);
    *header = (0 << EEM_DATA_CRC_SHIFT) | ((length + sizeof(uint32_t)) & EEM_DATA_LEN_MASK);
    memcpy(tx_buffer_ + sizeof(uint16_t), buffer, length);
    memcpy(tx_buffer_ + sizeof(uint16_t) + length, &crc, sizeof(uint32_t));
    status = USB_DeviceCdcEemSend(class_handle_, bulk_in_ep_, tx_buffer_, sizeof(uint16_t) + length + sizeof(uint32_t));
    if (status != kStatus_USB_Success) {
        return ERR_IF;
    }

    if (xSemaphoreTake(tx_semaphore_, pdMS_TO_TICKS(200)) == pdFALSE) {
        status = kStatus_USB_Error;
    }

    return (status == kStatus_USB_Success) ? ERR_OK : ERR_IF;
}

err_t CdcEem::ReceiveFrame(uint8_t *buffer, uint32_t length) {
    struct netif *tmp_netif;
    for (tmp_netif = netif_list; (tmp_netif != NULL) && (tmp_netif->state != this); tmp_netif = tmp_netif->next) {}
    if (!tmp_netif) {
        printf("Couldn't find EEM interface\r\n");
        return ERR_IF;
    }

    struct pbuf *frame = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
    if (!frame) {
        printf("Failed to allocate pbuf\r\n");
        return ERR_BUF;
    }

    memcpy(frame->payload, buffer, length);
    err_t ret = tcpip_input(frame, tmp_netif);
    if (ret != ERR_OK) {
        pbuf_free(frame);
        return ERR_IF;
    }

    return ERR_OK;
}

usb_status_t CdcEem::Transmit(uint8_t* buffer, uint32_t length) {
    usb_status_t status;
    if (length > 512) {
        assert(false);
    }
    memcpy(tx_buffer_, buffer, length);
    status = USB_DeviceCdcEemSend(class_handle_, bulk_in_ep_, tx_buffer_, length);
    if (status != kStatus_USB_Success) {
        return status;
    }

    if (xSemaphoreTake(tx_semaphore_, pdMS_TO_TICKS(200)) == pdFALSE) {
        status = kStatus_USB_Error;
    }

    return status;
}

usb_status_t CdcEem::SendEchoRequest() {
    uint8_t echo_size = 16;
    uint8_t echo_buffer[echo_size + sizeof(uint16_t)];

    uint16_t *command = (uint16_t*)echo_buffer;
    *command = (1 << EEM_HEADER_TYPE_SHIFT) | (EEM_COMMAND_ECHO << EEM_COMMAND_OPCODE_SHIFT) | echo_size;
    uint8_t *echo_data = echo_buffer + sizeof(uint16_t);
    for (int i = 0; i < echo_size; ++i) {
        echo_data[i] = i;
    }

    return Transmit(echo_buffer, sizeof(echo_buffer));
}

void CdcEem::ProcessPacket() {
    uint16_t packet_hdr = *((uint16_t*)rx_buffer_);
    if (packet_hdr & EEM_HEADER_TYPE_MASK) {
        uint16_t opcode = (packet_hdr & EEM_COMMAND_OPCODE_MASK) >> EEM_COMMAND_OPCODE_SHIFT;
        uint16_t param = (packet_hdr & EEM_COMMAND_PARAM_MASK) >> EEM_COMMAND_PARAM_SHIFT;
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
        uint8_t *data = rx_buffer_ + sizeof(uint16_t);
        uint16_t data_len = len - sizeof(uint32_t);
        // TODO(atv): We should validate checksum. But we won't (for now). See if the stack handles that?
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
            initialized_ = true;
            USB_DeviceCdcEemRecv(class_handle_, bulk_out_ep_, rx_buffer_, cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize);
            break;
        default:
            DbgConsole_Printf("%s unhandled event %d\r\n", __PRETTY_FUNCTION__, event);
            return false;
    }
    return (status == kStatus_USB_Success);
}

usb_status_t CdcEem::Handler(uint32_t event, void *param) {
    usb_status_t ret = kStatus_USB_Error;
    usb_device_endpoint_callback_message_struct_t *ep_cb = (usb_device_endpoint_callback_message_struct_t*)param;

    switch (event) {
        case kUSB_DeviceEemEventRecvResponse: {
            ProcessPacket();
            ret = USB_DeviceCdcEemRecv(class_handle_, bulk_out_ep_, rx_buffer_, cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize);
            break;
        }
        case kUSB_DeviceEemEventSendResponse:
            if (ep_cb->length != 0 && (ep_cb->length % cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize) == 0) {
                ret = USB_DeviceCdcEemSend(class_handle_, bulk_in_ep_, nullptr, 0);
            } else {
                if (ep_cb->buffer || (!ep_cb->buffer && ep_cb->length == 0)) {
                    xSemaphoreGive(tx_semaphore_);
                    ret = USB_DeviceCdcEemRecv(class_handle_, bulk_out_ep_, rx_buffer_, cdc_eem_data_endpoints_[DATA_OUT].maxPacketSize);
                }
            }
            break;
        default:
            DbgConsole_Printf("Unhandled EEM event: %d\r\n", event);
    }

    return ret;
}

usb_status_t CdcEem::Handler(class_handle_t class_handle, uint32_t event, void *param) {
    return handle_map_[class_handle]->Handler(event, param);
}

}  // namespace valiant
