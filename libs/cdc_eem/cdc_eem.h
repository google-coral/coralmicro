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

#ifndef LIBS_CDC_EEM_CDC_EEM_H_
#define LIBS_CDC_EEM_CDC_EEM_H_

#include "libs/usb/descriptors.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h" // Must be above other class headers.
#include "libs/nxp/rt1176-sdk/usb_device_cdc_eem.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/netifapi.h"

#include <map>

namespace coral::micro {

class CdcEem {
  public:
    CdcEem() = default;
    CdcEem(const CdcEem&) = delete;
    CdcEem& operator=(const CdcEem&) = delete;
    void Init(uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t data_iface);
    const usb_device_class_config_struct_t& config_data() const { return config_; }
    const void *descriptor_data() const { return &descriptor_; }
    size_t descriptor_data_size() const { return sizeof(descriptor_); }
    void SetClassHandle(class_handle_t class_handle) {
        handle_map_[class_handle] = this;
        class_handle_ = class_handle;
    }
    bool HandleEvent(uint32_t event, void *param);

  private:
    usb_status_t SetControlLineState(usb_device_cdc_eem_request_param_struct_t* eem_param);
    void ProcessPacket(uint32_t packet_length);

    static std::map<class_handle_t, CdcEem*> handle_map_;
    static usb_status_t StaticHandler(class_handle_t class_handle, uint32_t event, void *param) {
        return handle_map_[class_handle]->Handler(event, param);
    }
    usb_status_t Handler(uint32_t event, void *param);

    // LwIP hooks
    static err_t StaticNetifInit(struct netif *netif) {
        return static_cast<CdcEem*>(netif->state)->NetifInit(netif);
    }
    err_t NetifInit(struct netif *netif);

    static err_t StaticTxFunc(struct netif *netif, struct pbuf *p) {
        return static_cast<CdcEem*>(netif->state)->TxFunc(netif, p);
    }
    err_t TxFunc(struct netif *netif, struct pbuf *p);

    static void StaticTaskFunction(void *param) {
        static_cast<CdcEem*>(param)->TaskFunction(param);
    }
    void TaskFunction(void *param);

    err_t TransmitFrame(void *buffer, uint32_t length);
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
        StaticHandler,
        nullptr,
        &class_struct_,
    };
    static constexpr CdcEemClassDescriptor descriptor_ = {
        {
            sizeof(InterfaceDescriptor),
            0x4,
            2,
            0, 2, 0x2, 0xC, 0x7, 0,
        }, // InterfaceDescriptor
        {
            sizeof(EndpointDescriptor),
            0x05,
            4 | 0x80, 0x02, 512, 0
        }, // EndpointDescriptor
        {
            sizeof(EndpointDescriptor),
            0x05,
            5 & 0x7F, 0x02, 512, 0
        }, // EndpointDescriptor
    }; // CdcEemClassDescriptor

    uint8_t serial_state_buffer_[10];
    uint8_t tx_buffer_[512];
    uint8_t rx_buffer_[512];
    uint8_t bulk_in_ep_, bulk_out_ep_;
    QueueHandle_t tx_queue_;
    class_handle_t class_handle_;

    ip4_addr_t netif_ipaddr_, netif_netmask_, netif_gw_;
    struct netif netif_;

    enum class Endianness {
        kUnknown,
        kLittleEndian,
        kBigEndian,
    };
    Endianness endianness_ = Endianness::kUnknown;
    void DetectEndianness(uint32_t packet_length);
};

}  // namespace coral::micro

#endif  // LIBS_CDC_EEM_CDC_EEM_H_
