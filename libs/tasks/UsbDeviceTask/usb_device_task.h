#ifndef _LIBS_TASKS_USBDEVICETASK_USBDEVICETASK_H_
#define _LIBS_TASKS_USBDEVICETASK_USBDEVICETASK_H_

#include "libs/usb/descriptors.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"

#include <array>
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

    void AddDevice(usb_device_class_config_struct_t* config, usb_set_handle_callback sh_cb, usb_handle_event_callback he_cb, void* descriptor_data, size_t descriptor_data_size);
    bool Init();
    static UsbDeviceTask* GetSingleton() {
        static UsbDeviceTask task;
        return &task;
    }
    void UsbDeviceTaskFn();

    uint8_t next_descriptor_value() { return ++next_descriptor_value_; }
    uint8_t next_interface_value() {
      uint8_t next_interface = next_interface_value_;
      CompositeDescriptor *p_composite_descriptor = reinterpret_cast<CompositeDescriptor*>(composite_descriptor_.data());
      p_composite_descriptor->conf.num_interfaces = ++next_interface_value_;
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
    std::array<uint8_t, 1024> composite_descriptor_;
    size_t composite_descriptor_size_ = 0;

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
};

}  // namespace valiant

#endif  // _LIBS_TASKS_USBDEVICETASK_USBDEVICETASK_H_
