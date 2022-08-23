/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_USB_USB_HOST_TASK_H_
#define LIBS_USB_USB_HOST_TASK_H_

#include <functional>
#include <map>

#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"

namespace coralmicro {

// Operates the Dev Board Micro as a USB host.
class UsbHostTask {
 public:
  UsbHostTask();
  UsbHostTask(const UsbHostTask&) = delete;
  UsbHostTask& operator=(const UsbHostTask&) = delete;

  static UsbHostTask* GetSingleton() {
    static UsbHostTask task;
    return &task;
  }
  // @cond Do not generate docs
  void Init();
  // @endcond

  using UsbHostEventCallback =
      std::function<usb_status_t(usb_host_handle, usb_device_handle,
                                 usb_host_configuration_handle, uint32_t)>;
  void RegisterUsbHostEventCallback(uint32_t vid, uint32_t pid,
                                    UsbHostEventCallback fn);
  usb_status_t HostEvent(usb_device_handle device_handle,
                         usb_host_configuration_handle config_handle,
                         uint32_t event_code);

  usb_host_handle host_handle() const { return host_handle_; }

 private:
  static void StaticTaskMain(void* param) {
    static_cast<UsbHostTask*>(param)->TaskMain();
  }
  void TaskMain();
  usb_host_handle host_handle_;

  // Map from VID/PID to callback
  // Key is VID in the top 16 bits, PID in the bottom 16 bits.
  std::map<uint32_t, UsbHostEventCallback> host_event_callbacks_;
};

}  // namespace coralmicro

#endif  // LIBS_USB_USB_HOST_TASK_H_
