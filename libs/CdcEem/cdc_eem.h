#ifndef __LIBS_CDCEEM_CDC_EEM_H_
#define __LIBS_CDCEEM_CDC_EEM_H_

#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h" // Must be above other class headers.
#include "libs/nxp/rt1176-sdk/usb_device_cdc_eem.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/netifapi.h"

#include <map>

namespace valiant {

class CdcEem {
  public:
    CdcEem();
    CdcEem(const CdcEem&) = delete;
    CdcEem& operator=(const CdcEem&) = delete;
    void Init(uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t data_iface);
    usb_device_class_config_struct_t* config_data() { return &config_; }
    void SetClassHandle(class_handle_t class_handle);
    bool HandleEvent(uint32_t event, void *param);
    usb_status_t Transmit(uint8_t *buffer, uint32_t length);
    bool Initialized() { return initialized_; }
  private:

    void ProcessPacket();
    usb_status_t SendEchoRequest();
    static usb_status_t Handler(class_handle_t class_handle, uint32_t event, void *param);
    usb_status_t Handler(uint32_t event, void *param);

    // LwIP hooks
    static err_t StaticNetifInit(struct netif *netif);
    err_t NetifInit(struct netif *netif);
    static err_t StaticTxFunc(struct netif *netif, struct pbuf *p);
    err_t TxFunc(struct netif *netif, struct pbuf *p);
    err_t TransmitFrame(uint8_t *buffer, uint32_t length);
    err_t ReceiveFrame(uint8_t *buffer, uint32_t length);

    usb_device_endpoint_struct_t cdc_eem_data_endpoints_[2] = {
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
    usb_device_interface_struct_t cdc_eem_data_interface_[1] = {
        {
            0,
            {
                ARRAY_SIZE(cdc_eem_data_endpoints_),
                cdc_eem_data_endpoints_,
            }
        },
    };
    usb_device_interfaces_struct_t cdc_eem_interfaces_[1] = {
        {
            0x02, // InterfaceClass
            0x0C, // InterfaceSubClass
            0x07, // InterfaceProtocol
            0,    // set in constructor
            cdc_eem_data_interface_,
            ARRAY_SIZE(cdc_eem_data_interface_),
        },
    };
    usb_device_interface_list_t cdc_eem_interface_list_[1] = {
        {
            ARRAY_SIZE(cdc_eem_interfaces_), cdc_eem_interfaces_,
        },
    };
    usb_device_class_struct_t class_struct_ {
        cdc_eem_interface_list_,
        kUSB_DeviceClassTypeEem,
        ARRAY_SIZE(cdc_eem_interface_list_),
    };
    usb_device_class_config_struct_t config_ {
        Handler,
        nullptr,
        &class_struct_,
    };

    bool initialized_ = false;
    uint8_t tx_buffer_[512];
    uint8_t rx_buffer_[512];
    uint8_t bulk_in_ep_, bulk_out_ep_;
    SemaphoreHandle_t tx_semaphore_;
    class_handle_t class_handle_;

    static std::map<class_handle_t, CdcEem*> handle_map_;

    ip4_addr_t netif_ipaddr_, netif_netmask_, netif_gw_;
    struct netif netif_;
};

}  // namespace valiant

#endif  // __LIBS_CDCEEM_CDC_EEM_H_
