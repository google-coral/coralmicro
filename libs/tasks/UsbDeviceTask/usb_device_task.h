#ifndef _LIBS_TASKS_USBDEVICETASK_USBDEVICETASK_H_
#define _LIBS_TASKS_USBDEVICETASK_USBDEVICETASK_H_

#include "libs/usb/descriptors.h"
#include "libs/CdcEem/cdc_eem.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"

#include <functional>
#include <vector>

namespace valiant {

using usb_set_handle_callback = std::function<void(class_handle_t)>;
using usb_handle_event_callback = std::function<bool(uint32_t, void*)>;
class UsbDeviceTask {
  public:
    UsbDeviceTask();
    UsbDeviceTask(const UsbDeviceTask&) = delete;
    UsbDeviceTask &operator=(const UsbDeviceTask&) = delete;

    void AddDevice(usb_device_class_config_struct_t* config, usb_set_handle_callback sh_cb, usb_handle_event_callback he_cb);
    bool Init();
    static UsbDeviceTask* GetSingleton() {
        static UsbDeviceTask task;
        return &task;
    }
    void UsbDeviceTaskFn();

    uint8_t next_descriptor_value() { return ++next_descriptor_value_; }
    uint8_t next_interface_value() {
      uint8_t next_interface = next_interface_value_;
      composite_descriptor_.conf.num_interfaces = ++next_interface_value_;
      return next_interface;
    }
    usb_device_handle device_handle() { return device_handle_; }
  private:
    uint8_t next_descriptor_value_ = 0;
    uint8_t next_interface_value_ = 0;
    DeviceDescriptor device_descriptor_ = {
        sizeof(DeviceDescriptor), 0x01, 0x0200,
        0xEF, 0x02, 0x01, 0x40,
        0x18d1, 0x93FF, 0x0001,
        1, 2, 3, 1
    };
    CompositeDescriptor composite_descriptor_ = {
        {
            sizeof(ConfigurationDescriptor),
            0x02,
            sizeof(CompositeDescriptor),
            0, // Managed by next_interface_value
            1,
            0,
            0x80, // kUsb11AndHigher
            250, // kUses500mA
        }, // ConfigurationDescriptor
        {
            sizeof(InterfaceAssociationDescriptor),
            0x0B,
            0, // first iface num
            1, // total num ifaces
            0x02, 0x02, 0x01, 0,
        }, // InterfaceAssociationDescriptor
        {
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
        }, // CdcAcmClassDescriptor
        {
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
        }, // CdcEemClassDescriptor
    };

    LangIdDescriptor lang_id_desc_ = {
        sizeof(LangIdDescriptor),
        0x03,
        0x0409,
    };

    static usb_status_t StaticHandler(usb_device_handle device_handle, uint32_t event, void *param);
    usb_status_t Handler(usb_device_handle device_handle, uint32_t event, void *param);

    std::vector<usb_device_class_config_struct_t> configs_;
    usb_device_class_config_list_struct_t config_list_;
    usb_device_handle device_handle_;

    std::vector<usb_set_handle_callback> set_handle_callbacks_;
    std::vector<usb_handle_event_callback> handle_event_callbacks_;

    CdcEem cdc_eem_;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_USBDEVICETASK_USBDEVICETASK_H_
