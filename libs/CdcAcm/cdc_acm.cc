#include "libs/CdcAcm/cdc_acm.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.h"

#include <cstdio>

namespace valiant {

std::map<class_handle_t, CdcAcm*> CdcAcm::handle_map_;

CdcAcm::CdcAcm(uint8_t interrupt_in_ep, uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t comm_iface, uint8_t data_iface,
        RxHandler rx_handler) :
    interrupt_in_ep_(interrupt_in_ep),
    bulk_in_ep_(bulk_in_ep),
    bulk_out_ep_(bulk_out_ep),
    rx_handler_(rx_handler) {
    cdc_acm_comm_endpoints_[0].endpointAddress = interrupt_in_ep | (USB_IN << 7);
    cdc_acm_data_endpoints_[0].endpointAddress = bulk_in_ep | (USB_IN << 7);
    cdc_acm_data_endpoints_[1].endpointAddress = bulk_out_ep | (USB_OUT << 7);
    cdc_acm_interfaces_[0].interfaceNumber = comm_iface;
    cdc_acm_interfaces_[1].interfaceNumber = data_iface;
    tx_semaphore_ = xSemaphoreCreateBinary();
}

void CdcAcm::SetClassHandle(class_handle_t class_handle) {
    handle_map_[class_handle] = this;
    class_handle_ = class_handle;
}

void CdcAcm::SetConfiguration() {
    usb_status_t status = USB_DeviceCdcAcmRecv(
            class_handle_,
            bulk_out_ep_,
            rx_buffer_,
            cdc_acm_data_endpoints_[1].maxPacketSize);
    if (status != kStatus_USB_Success) {
        printf("USB_DeviceCdcAcmRecv failed in %s: %d\r\n", __PRETTY_FUNCTION__, status);
    }
}

bool CdcAcm::Transmit(const uint8_t *buffer, const size_t length) {
    usb_status_t status;
    if (length > 512) {
        printf("%s data larger than tx_buffer_\r\n", __PRETTY_FUNCTION__);
        return false;
    }

    memcpy(tx_buffer_, buffer, length);
    status = USB_DeviceCdcAcmSend(
            class_handle_, bulk_in_ep_, tx_buffer_, length);

    if (status != kStatus_USB_Success) {
        printf("%s: USB_DeviceCdcAcmSend failed: %d\r\n", __func__, status);
        return false;
    }
    if (xSemaphoreTake(tx_semaphore_, pdMS_TO_TICKS(200)) == pdTRUE) {
        return true;
    } else {
        return false;
    }
}

bool CdcAcm::HandleEvent(uint32_t event, void *param) {
    switch (event) {
        case kUSB_DeviceEventSetConfiguration:
            SetConfiguration();
            break;
        default:
            printf("%s: unhandled event %lu\r\n", __PRETTY_FUNCTION__, event);
            return false;
    }
    return true;
}

usb_status_t CdcAcm::Handler(uint32_t event, void *param) {
    usb_status_t ret = kStatus_USB_Error;
    usb_device_endpoint_callback_message_struct_t* ep_cb = (usb_device_endpoint_callback_message_struct_t*)param;
    switch (event) {
        case kUSB_DeviceCdcEventSendResponse:
            if ((ep_cb->length != 0) && (ep_cb->length % cdc_acm_data_endpoints_[1].maxPacketSize) == 0) {
                // The packet is equal to the size of the endpoint. Send a zero length packet
                // to let the other side know we're done.
                ret = USB_DeviceCdcAcmSend(class_handle_, bulk_in_ep_, nullptr, 0);
            } else {
                if (ep_cb->buffer || (!ep_cb->buffer && ep_cb->length == 0)) {
                    xSemaphoreGive(tx_semaphore_);
                    ret = USB_DeviceCdcAcmRecv(
                            class_handle_,
                            bulk_out_ep_,
                            rx_buffer_,
                            cdc_acm_data_endpoints_[1].maxPacketSize);
                }
            }
            break;
        case kUSB_DeviceCdcEventRecvResponse:
            if (rx_handler_) {
                rx_handler_(rx_buffer_, ep_cb->length);
            }
            ret = USB_DeviceCdcAcmRecv(
                    class_handle_,
                    bulk_out_ep_,
                    rx_buffer_,
                    cdc_acm_data_endpoints_[1].maxPacketSize);
            break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t*)class_handle_)->hasSentState = 0;
            ret = kStatus_USB_Success;
            break;
        default:
            printf("%s: unhandled event %lu\r\n", __PRETTY_FUNCTION__, event);
    }
    return ret;
}

usb_status_t CdcAcm::Handler(class_handle_t class_handle, uint32_t event, void *param) {
    return handle_map_[class_handle]->Handler(event, param);
}

}  // namespace valiant
