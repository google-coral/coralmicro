#include "libs/CdcEem/cdc_eem.h"
#include "libs/nxp/rt1176-sdk/usb_device_cdc_eem.h"

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
}

void CdcEem::SetClassHandle(class_handle_t class_handle) {
    handle_map_[class_handle] = this;
    class_handle_ = class_handle;
}

bool CdcEem::TransmitFrame(uint8_t* buffer, uint32_t length) {
    usb_status_t status;
    uint32_t crc = FreeRTOS_htonl(0xdeadbeef);
    uint16_t *header = (uint16_t*)tx_buffer_;
    *header = (0 << EEM_DATA_CRC_SHIFT) | ((length + sizeof(uint32_t)) & EEM_DATA_LEN_MASK);
    memcpy(tx_buffer_ + sizeof(uint16_t), buffer, length);
    memcpy(tx_buffer_ + sizeof(uint16_t) + length, &crc, sizeof(uint32_t));
    status = USB_DeviceCdcEemSend(class_handle_, bulk_in_ep_, tx_buffer_, sizeof(uint16_t) + length + sizeof(uint32_t));
    if (status != kStatus_USB_Success) {
        return false;
    }

    if (xSemaphoreTake(tx_semaphore_, pdMS_TO_TICKS(200)) == pdFALSE) {
        status = kStatus_USB_Error;
    }

    return (status == kStatus_USB_Success);
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
        NetworkInterface::ReceiveFrame(data, data_len);
    }
}

bool CdcEem::HandleEvent(uint32_t event, void *param) {
    usb_status_t status;
    switch (event) {
        case kUSB_DeviceEventSetConfiguration:
            break;
        case kUSB_DeviceEventSetInterface:
            initialized_ = true;
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
