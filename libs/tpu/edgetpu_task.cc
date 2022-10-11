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

#include "libs/tpu/edgetpu_task.h"

#include <cstdio>
#include <functional>

#include "libs/base/gpio.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/usb_host_edgetpu.h"
#include "libs/usb/usb_host_task.h"

using namespace std::placeholders;

namespace coralmicro {
using namespace edgetpu;

void EdgeTpuTask::SetNextState(EdgeTpuState next_state) {
  Request req;
  req.type = RequestType::kNextState;
  req.request.next_state.state = next_state;
  SendRequestAsync(req);
}

usb_status_t EdgeTpuTask::USBHostEvent(
    usb_host_handle host_handle, usb_device_handle device_handle,
    usb_host_configuration_handle config_handle, uint32_t event_code) {
  usb_host_configuration_t *configuration_ptr;
  uint8_t usb_status = event_code >> 16;
  switch (event_code & 0xFFFF) {
    case kUSB_HostEventAttach:
      configuration_ptr =
          reinterpret_cast<usb_host_configuration_t *>(config_handle);
      for (unsigned int i = 0; i < configuration_ptr->interfaceCount; ++i) {
        auto *interface_ptr = &configuration_ptr->interfaceList[i];
        uint8_t id = interface_ptr->interfaceDesc->bInterfaceClass;

        if (id != USB_HOST_EDGETPU_CLASS_CODE) {
          continue;
        }

        id = interface_ptr->interfaceDesc->bInterfaceSubClass;
        if (id == USB_HOST_EDGETPU_SUBCLASS_CODE) {
          SetDeviceHandle(device_handle);
          SetInterfaceHandle(interface_ptr);
          break;
        }
      }
      return (this->device_handle() != nullptr) ? kStatus_USB_Success
                                                : kStatus_USB_NotSupported;
    case kUSB_HostEventEnumerationDone:
      SetNextState(EdgeTpuState::kAttached);
      return kStatus_USB_Success;
    case kUSB_HostEventDetach:
#if 0
      printf("EdgeTPU went away...\r\n");
#endif
      SetNextState(EdgeTpuState::kUnattached);
      if (auto status = USB_HostEdgeTpuDeinit(this->device_handle(),
                                              this->class_handle());
          status != kStatus_USB_Success) {
        printf("USB_HostEdgeTpuDeinit error: %u\r\n", status);
      }
      if (auto status =
              USB_HostRemoveDevice(host_handle, this->device_handle()) !=
              kStatus_USB_Success) {
        printf("USB_HostRemoveDevice error: %u\r\n", status);
      }
      return kStatus_USB_Success;
    case kUSB_HostEventEnumerationFail:
      printf("%s enumeration failed: %u\r\n", __func__, usb_status);
      SetNextState(EdgeTpuState::kEnumerationFailed);
      return static_cast<usb_status_t>(usb_status);
    default:
      if (usb_status != kStatus_USB_Success) {
        printf("%s failed: %u\r\n", __func__, usb_status);
        SetNextState(EdgeTpuState::kError);
      }
      return static_cast<usb_status_t>(usb_status);
  }
}

void EdgeTpuTask::SetInterfaceCallback(void *param, uint8_t *, uint32_t,
                                       usb_status_t status) {
  auto *task = static_cast<EdgeTpuTask *>(param);
  if (status != kStatus_USB_Success) {
    printf("Error in EdgeTpuSetInterface\r\n");
    task->SetNextState(EdgeTpuState::kError);
    return;
  }
  task->SetNextState(EdgeTpuState::kGetStatus);
}

void EdgeTpuTask::GetStatusCallback(void *param, uint8_t *, uint32_t,
                                    usb_status_t status) {
  auto *task = static_cast<EdgeTpuTask *>(param);
  if (status != kStatus_USB_Success) {
    printf("Error in EdgeTpuGetStatus\r\n");
    task->SetNextState(EdgeTpuState::kError);
    return;
  }
  task->SetNextState(EdgeTpuState::kConnected);
}

void EdgeTpuTask::TaskInit() {
  coralmicro::UsbHostTask::GetSingleton()->RegisterUsbHostEventCallback(
      kEdgeTpuVid, kEdgeTpuPid,
      std::bind(&EdgeTpuTask::USBHostEvent, this, _1, _2, _3, _4));
}

bool EdgeTpuTask::HandleGetPowerRequest() const { return (enabled_count_ > 0); }

void EdgeTpuTask::HandleSetPowerRequest(SetPowerRequest &req) {
  if (req.enable) {
    enabled_count_++;
    // Already enabled, increment count and exit.
    if (enabled_count_ > 1) {
      return;
    }
  } else {
    enabled_count_ = std::max(0, enabled_count_ - 1);
    // Someone else still asked to keep this on, exit.
    if (enabled_count_ > 0) {
      return;
    }
  }

  GpioSet(Gpio::kEdgeTpuPmic, req.enable);
  if (req.enable) {
    bool power_good{false};
    while (!power_good) {
      power_good = GpioGet(Gpio::kEdgeTpuPgood);
      taskYIELD();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  GpioSet(Gpio::kEdgeTpuReset, req.enable);
}

void EdgeTpuTask::HandleNextState(NextStateRequest &req) {
  usb_status_t ret;
  EdgeTpuState next_state = req.state;

  switch (next_state) {
    case EdgeTpuState::kUnattached:
      EdgeTpuManager::GetSingleton()->NotifyConnected(nullptr);
      break;
    case EdgeTpuState::kAttached:
      ret = USB_HostEdgeTpuInit(device_handle(), &class_handle_);
      if (ret != kStatus_USB_Success) {
        SetNextState(EdgeTpuState::kError);
      } else {
        SetNextState(EdgeTpuState::kSetInterface);
      }
      break;
    case EdgeTpuState::kSetInterface:
      ret = USB_HostEdgeTpuSetInterface(class_handle(), interface_handle(), 0,
                                        SetInterfaceCallback, this);
      if (ret != kStatus_USB_Success) {
        SetNextState(EdgeTpuState::kError);
      }
      // Thunderchild notifies EdgeTpuManager that it's connected. Should this
      // be in callback?
      EdgeTpuManager::GetSingleton()->NotifyConnected(
          reinterpret_cast<usb_host_edgetpu_instance_t *>(class_handle()));
      break;
    case EdgeTpuState::kGetStatus:
      ret = USB_HostEdgeTpuGetStatus(class_handle(), &status_,
                                     GetStatusCallback, this);
      if (ret != kStatus_USB_Success) {
        SetNextState(EdgeTpuState::kError);
      }
      break;
    case EdgeTpuState::kConnected:
      break;
    case EdgeTpuState::kEnumerationFailed:
    case EdgeTpuState::kError:
    default:
      EdgeTpuManager::GetSingleton()->NotifyError();
      break;
  }
}

bool EdgeTpuTask::GetPower() {
  Request req;
  req.type = RequestType::kGetPower;
  Response resp = SendRequest(req);
  return resp.response.get_power.enabled;
}

void EdgeTpuTask::SetPower(bool enable) {
  Request req;
  req.type = RequestType::kSetPower;
  req.request.set_power.enable = enable;
  SendRequest(req);
}

void EdgeTpuTask::RequestHandler(Request *req) {
  Response resp;
  resp.type = req->type;
  switch (req->type) {
    case RequestType::kNextState:
      HandleNextState(req->request.next_state);
      break;
    case RequestType::kSetPower:
      HandleSetPowerRequest(req->request.set_power);
      break;
    case RequestType::kGetPower:
      resp.response.get_power.enabled = HandleGetPowerRequest();
      break;
  }

  if (req->callback) {
    req->callback(resp);
  }
}

}  // namespace coralmicro
