#ifndef LIBS_CDC_ACM_CDC_ACM_H_
#define LIBS_CDC_ACM_CDC_ACM_H_

#include "libs/usb/descriptors.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h" // Must be above other class headers.
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.h"

#include <functional>
#include <map>

typedef struct _usb_host_cdc_line_coding_struct
{
    uint32_t dwDTERate;  /*!< Data terminal rate, in bits per second*/
    uint8_t bCharFormat; /*!< Stop bits*/
    uint8_t bParityType; /*!< Parity*/
    uint8_t bDataBits;   /*!< Data bits (5, 6, 7, 8 or 16).*/
} __attribute__((packed)) usb_host_cdc_line_coding_struct_t;

namespace coral::micro {

class CdcAcm {
  public:
    using RxHandler = std::function<void(const uint8_t*, const uint32_t)>;

    CdcAcm() = default;
    CdcAcm(const CdcAcm&) = delete;
    CdcAcm& operator=(const CdcAcm&) = delete;
    void Init(uint8_t interrupt_in_ep, uint8_t bulk_in_ep, uint8_t bulk_out_ep, uint8_t comm_iface, uint8_t data_iface, RxHandler rx_handler);
    const usb_device_class_config_struct_t& config_data() const { return config_; }
    const void *descriptor_data() const { return &descriptor_; }
    size_t descriptor_data_size() const { return sizeof(descriptor_); }
    // TODO(atv): Make me private
    void SetClassHandle(class_handle_t class_handle) {
        handle_map_[class_handle] = this;
        class_handle_ = class_handle;
    }
    bool HandleEvent(uint32_t event, void *param);
    bool Transmit(const uint8_t *buffer, const size_t length);
  private:
    bool can_transmit_ = false;
    void SetConfiguration();
    usb_status_t SetControlLineState(usb_device_cdc_acm_request_param_struct_t* acm_param);

    static std::map<class_handle_t, CdcAcm*> handle_map_;
    static usb_status_t StaticHandler(class_handle_t class_handle, uint32_t event, void *param) {
        return handle_map_[class_handle]->Handler(event, param);
    }
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
        StaticHandler,
        nullptr,
        &class_struct_,
    };
    static constexpr CdcAcmClassDescriptor descriptor_ = {
        {
            sizeof(InterfaceAssociationDescriptor),
            0x0B,
            0, // first iface num
            2, // total num ifaces
            0x02, 0x02, 0x01, 0,
        }, // InterfaceAssociationDescriptor
        {
            sizeof(InterfaceDescriptor),
            0x04,
            0, // iface num
            0, 1, 0x02, 0x02, 0x01, 0,
        }, // InterfaceDescriptor
        {
            sizeof(CdcHeaderFunctionalDescriptor),
            0x24,
            0x00, 0x0110,
        }, // CdcHeaderFunctionalDescriptor
        {
            sizeof(CdcCallManagementFunctionalDescriptor),
            0x24,
            0x01, 0, 1,
        }, // CdcCallManagementFunctionalDescriptor
        {
            sizeof(CdcAcmFunctionalDescriptor),
            0x24,
            0x02, 2,
        }, // CdcAcmFunctionalDescriptor
        {
            sizeof(CdcUnionFunctionalDescriptor),
            0x24,
            0x06, 0, 1
        }, // CdcUnionFunctionalDescriptor
        {
            sizeof(EndpointDescriptor),
            0x05,
            1 | 0x80, 0x03, 10, 9,
        }, // EndpointDescriptor

        {
            sizeof(InterfaceDescriptor),
            0x04,
            1,
            0, 2, 0x0A, 0x00, 0x00, 0,
        }, // InterfaceDescriptor
        {
            sizeof(EndpointDescriptor),
            0x05,
            2 | 0x80, 0x02, 512, 0,
        }, // EndpointDescriptor
        {
            sizeof(EndpointDescriptor),
            0x05,
            3 & 0x7F, 0x02, 512, 0,
        }, // EndpointDescriptor
    }; // CdcAcmClassDescriptor

    uint8_t tx_buffer_[512];
    uint8_t rx_buffer_[512];
    uint8_t serial_state_buffer_[10];
    SemaphoreHandle_t tx_semaphore_;
    uint8_t interrupt_in_ep_, bulk_in_ep_, bulk_out_ep_;
    RxHandler rx_handler_;
    class_handle_t class_handle_;
    usb_host_cdc_line_coding_struct_t line_coding_;
};

}  // namespace coral::micro

#endif  // LIBS_CDC_ACM_CDC_ACM_H_
