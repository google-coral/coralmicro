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

#ifndef LIBS_TPU_EDGETPU_TASK_H_
#define LIBS_TPU_EDGETPU_TASK_H_

#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/modified/nxp/rt1176-sdk/usb_host_config.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"

namespace coralmicro {

inline constexpr int kEdgeTpuVid = 0x18d1;
inline constexpr int kEdgeTpuPid = 0x9302;

namespace edgetpu {

enum class EdgeTpuState : uint8_t {
  kUnattached,
  kAttached,
  kSetInterface,
  kGetStatus,
  kConnected,
  kEnumerationFailed,
  kError,
};

enum class RequestType : uint8_t {
  kNextState,
  kSetPower,
  kGetPower,
};

struct NextStateRequest {
  EdgeTpuState state;
};

struct SetPowerRequest {
  bool enable;
};

struct GetPowerResponse {
  bool enabled;
};

struct Response {
  RequestType type;
  union {
    GetPowerResponse get_power;
  } response;
};

struct Request {
  RequestType type;
  union {
    NextStateRequest next_state;
    SetPowerRequest set_power;
  } request;
  std::function<void(Response)> callback;
};

}  // namespace edgetpu

inline constexpr char kEdgeTpuTaskName[] = "edgetpu_task";

class EdgeTpuTask
    : public QueueTask<edgetpu::Request, edgetpu::Response, kEdgeTpuTaskName,
                       configMINIMAL_STACK_SIZE * 3, kEdgeTpuTaskPriority,
                       /*QueueLength=*/4> {
 public:
  bool GetPower();
  void SetPower(bool enable);
  static EdgeTpuTask *GetSingleton() {
    static EdgeTpuTask task;
    return &task;
  }

  void SetDeviceHandle(usb_device_handle handle) { device_handle_ = handle; }
  usb_device_handle device_handle() const { return device_handle_; }

  void SetInterfaceHandle(usb_host_interface_handle handle) {
    interface_handle_ = handle;
  }
  usb_host_interface_handle interface_handle() const {
    return interface_handle_;
  }

  void SetClassHandle(usb_host_class_handle handle) { class_handle_ = handle; }
  usb_host_class_handle class_handle() const { return class_handle_; }

 private:
  void TaskInit() override;
  void RequestHandler(edgetpu::Request *req) override;
  void HandleNextState(edgetpu::NextStateRequest &req);
  bool HandleGetPowerRequest() const;
  void HandleSetPowerRequest(edgetpu::SetPowerRequest &req);
  void SetNextState(edgetpu::EdgeTpuState next_state);
  static void SetInterfaceCallback(void *param, uint8_t *data,
                                   uint32_t data_length, usb_status_t status);
  static void GetStatusCallback(void *param, uint8_t *data,
                                uint32_t data_length, usb_status_t status);
  usb_status_t USBHostEvent(usb_host_handle host_handle,
                            usb_device_handle device_handle,
                            usb_host_configuration_handle config_handle,
                            uint32_t event_code);

  usb_device_handle device_handle_;
  usb_host_interface_handle interface_handle_;
  usb_host_class_handle class_handle_;
  uint8_t status_;
  int enabled_count_ = 0;
};
}  // namespace coralmicro

#endif  // LIBS_TPU_EDGETPU_TASK_H_
