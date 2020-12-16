#ifndef __LIBS_CDCACM_CDC_ACM_H_
#define __LIBS_CDCACM_CDC_ACM_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"

#include <functional>
#include <map>

namespace valiant {


class CdcAcm {
  using RxHandler = std::function<void(const uint8_t*, const uint32_t)>;
  public:
    CdcAcm(uint8_t interrupt_in_ep, uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t comm_iface, uint8_t data_iface, RxHandler rx_handler);
    CdcAcm(const CdcAcm&) = delete;
    CdcAcm& operator=(const CdcAcm&) = delete;
    usb_device_class_config_struct_t* config_data() { return &config_; }
    // TODO(atv): Make me private
    void SetClassHandle(class_handle_t class_handle);
    bool HandleEvent(uint32_t event, void *param);
    bool Transmit(const uint8_t *buffer, const size_t length);
  private:
    void SetConfiguration();
    static usb_status_t Handler(class_handle_t class_handle, uint32_t event, void *param);
    usb_status_t Handler(uint32_t event, void *param);
    usb_device_endpoint_struct_t cdc_acm_comm_endpoints_[1] = {
        {
            0, // set in constructor
            USB_ENDPOINT_INTERRUPT,
            8, // max packet size
        },
    };
    usb_device_endpoint_struct_t cdc_acm_data_endpoints_[2] = {
        {
            0, // set in constructor
            USB_ENDPOINT_BULK,
            512,
        },
        {
            0, // set in constructor
            USB_ENDPOINT_BULK,
            512,
        },
    };
    usb_device_interface_struct_t cdc_acm_comm_interface_[1] = {
        {
            0,
            {
                ARRAY_SIZE(cdc_acm_comm_endpoints_),
                cdc_acm_comm_endpoints_,
            },
        },
    };
    usb_device_interface_struct_t cdc_acm_data_interface_[1] = {
        {
            0,
            {
                ARRAY_SIZE(cdc_acm_data_endpoints_),
                cdc_acm_data_endpoints_,
            }
        },
    };
    usb_device_interfaces_struct_t cdc_acm_interfaces_[2] = {
        // comm
        {
            0x02, // InterfaceClass
            0x02, // InterfaceSubClass
            0x00, // InterfaceProtocol
            0,    // Interface index  set in constructor
            cdc_acm_comm_interface_,
            ARRAY_SIZE(cdc_acm_comm_interface_),
        },
        // Data
        {
            0x0A, // InterfaceClass
            0x00, // InterfaceSubClass
            0x00, // InterfaceProtocol
            0,    // interface index set in constructor
            cdc_acm_data_interface_,
            ARRAY_SIZE(cdc_acm_data_interface_),
        },
    };
    usb_device_interface_list_t cdc_acm_interface_list_[1] = {
        {
            ARRAY_SIZE(cdc_acm_interfaces_), cdc_acm_interfaces_,
        },
    };
    usb_device_class_struct_t class_struct_ {
        cdc_acm_interface_list_,
        kUSB_DeviceClassTypeCdc,
        ARRAY_SIZE(cdc_acm_interface_list_),
    };
    usb_device_class_config_struct_t config_ {
        Handler,
        nullptr,
        &class_struct_,
    };

    uint8_t tx_buffer_[512];
    uint8_t rx_buffer_[512];
    SemaphoreHandle_t tx_semaphore_;
    uint8_t interrupt_in_ep_, bulk_in_ep_, bulk_out_ep_;
    RxHandler rx_handler_;
    class_handle_t class_handle_;

    static std::map<class_handle_t, CdcAcm*> handle_map_;
};

}  // namespace valiant

#endif  // __LIBS_CDCACM_CDC_ACM_H_
