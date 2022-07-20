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

#ifndef LIBS_TPU_EDGETPU_DFU_TASK_H_
#define LIBS_TPU_EDGETPU_DFU_TASK_H_

#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/modified/nxp/rt1176-sdk/usb_host_config.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"

extern unsigned char apex_latest_single_ep_bin[];
extern unsigned int apex_latest_single_ep_bin_len;

namespace coralmicro {

inline constexpr int kDfuVid = 0x1A6E;
inline constexpr int kDfuPid = 0x089A;

namespace edgetpu_dfu {

enum class DfuState : uint8_t {
  kUnattached,
  kAttached,
  kSetInterface,
  kGetStatus,
  kTransfer,
  kZeroLengthTransfer,
  kReadBack,
  kGetStatusRead,
  kDetach,
  kCheckStatus,
  kComplete,
  kError,
};

enum class RequestType : uint8_t {
  kNextState,
};

struct NextStateRequest {
  DfuState state;
};

struct Response {
  RequestType type;
};

struct Request {
  RequestType type;
  union {
    NextStateRequest next_state;
  } request;
  std::function<void(Response)> callback;
};

}  // namespace edgetpu_dfu

inline constexpr char kEdgeTpuDfuTaskName[] = "edgetpu_dfu_task";

class EdgeTpuDfuTask
    : public QueueTask<edgetpu_dfu::Request, edgetpu_dfu::Response,
                       kEdgeTpuDfuTaskName, configMINIMAL_STACK_SIZE * 3,
                       kEdgeTpuDfuTaskPriority, /*QueueLength=*/4> {
 public:
  static EdgeTpuDfuTask *GetSingleton() {
    static EdgeTpuDfuTask task;
    return &task;
  }

  // USB event handler
  usb_status_t USB_DFUHostEvent(usb_host_handle host_handle,
                                usb_device_handle device_handle,
                                usb_host_configuration_handle config_handle,
                                uint32_t event_code);

  // Callback methods for interacting with the USB stack
  static void SetInterfaceCallback(void *param, uint8_t *data,
                                   uint32_t data_length, usb_status_t status);
  static void GetStatusCallback(void *param, uint8_t *data,
                                uint32_t data_length, usb_status_t status);
  static void TransferCallback(void *param, uint8_t *data, uint32_t data_length,
                               usb_status_t status);
  static void ZeroLengthTransferCallback(void *param, uint8_t *data,
                                         uint32_t data_length,
                                         usb_status_t status);
  static void ReadBackCallback(void *param, uint8_t *data, uint32_t data_length,
                               usb_status_t status);
  static void GetStatusReadCallback(void *param, uint8_t *data,
                                    uint32_t data_length, usb_status_t status);
  static void DetachCallback(void *param, uint8_t *data, uint32_t data_length,
                             usb_status_t status);
  static void CheckStatusCallback(void *param, uint8_t *data,
                                  uint32_t data_length, usb_status_t status);

  // Getters/setters for members
  void SetHostInstance(usb_host_instance_t *instance) {
    host_instance_ = instance;
  }
  const usb_host_instance_t *host_instance() const { return host_instance_; }

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

  void SetStatus(usb_host_dfu_status_t status) { status_ = status; }
  usb_host_dfu_status_t status() const { return status_; }

  void SetBytesTransferred(size_t bytes) { bytes_transferred_ = bytes; }
  size_t bytes_transferred() const { return bytes_transferred_; }

  void SetBytesToTransfer(size_t bytes) { bytes_to_transfer_ = bytes; }
  size_t bytes_to_transfer() const { return bytes_to_transfer_; }

  void SetCurrentBlockNumber(size_t block) { current_block_number_ = block; }
  size_t current_block_number() const { return current_block_number_; }

  void SetReadBackData(uint8_t *read_back_data) {
    read_back_data_ = read_back_data;
  }
  uint8_t *read_back_data() { return read_back_data_; }

 private:
  void TaskInit() override;
  void RequestHandler(edgetpu_dfu::Request *req) override;
  void HandleNextState(edgetpu_dfu::NextStateRequest &req);
  void SetNextState(edgetpu_dfu::DfuState next_state);

  usb_host_instance_t *host_instance_;
  usb_device_handle device_handle_;
  usb_host_interface_handle interface_handle_;
  usb_host_class_handle class_handle_;
  usb_host_dfu_status_t status_;
  size_t bytes_transferred_ = 0;
  size_t bytes_to_transfer_ = apex_latest_single_ep_bin_len;
  size_t current_block_number_ = 0;
  uint8_t *read_back_data_ = nullptr;
};

}  // namespace coralmicro

#endif  // LIBS_TPU_EDGETPU_DFU_TASK_H_
