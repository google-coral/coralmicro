#ifndef _LIBS_TASKS_USBHOSTTASK_USBHOSTTASK_H_
#define _LIBS_TASKS_USBHOSTTASK_USBHOSTTASK_H_

#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"

#include <functional>
#include <map>

namespace coral::micro {

class UsbHostTask {
  public:
    UsbHostTask();
    UsbHostTask(const UsbHostTask&) = delete;
    UsbHostTask &operator=(const UsbHostTask&) = delete;

    static UsbHostTask* GetSingleton() {
      static UsbHostTask task;
        return &task;
    }
    void Init();

    using usb_host_event_callback = std::function<usb_status_t(usb_host_handle, usb_device_handle, usb_host_configuration_handle, uint32_t)>;
    void RegisterUSBHostEventCallback(uint32_t vid, uint32_t pid, usb_host_event_callback fn);
    usb_status_t HostEvent(usb_device_handle device_handle,
                           usb_host_configuration_handle config_handle,
                           uint32_t event_code);

    usb_host_handle host_handle() {
      return host_handle_;
    }
  private:
    static void StaticTaskMain(void *param);
    void TaskMain();
    usb_host_handle host_handle_;

    // Map from VID/PID to callback
    // Key is VID in the top 16 bits, PID in the bottom 16 bits.
    std::map<uint32_t, usb_host_event_callback> host_event_callbacks_;
};

}  // namespace coral::micro

#endif  // _LIBS_TASKS_USBHOSTTASK_USBHOSTTASK_H_
