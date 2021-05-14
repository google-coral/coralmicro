#ifndef _LIBS_TASKS_EDGETPUDFUTASK_EDGETPUDFUTASK_H_
#define _LIBS_TASKS_EDGETPUDFUTASK_EDGETPUDFUTASK_H_

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "libs/nxp/rt1176-sdk/usb_host_config.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include <functional>

extern unsigned char apex_latest_single_ep_bin[];
extern unsigned int apex_latest_single_ep_bin_len;

namespace valiant {

constexpr int kDfuVid = 0x1A6E;
constexpr int kDfuPid = 0x089A;

namespace edgetpu_dfu {

enum dfu_state {
    DFU_STATE_UNATTACHED = 0,
    DFU_STATE_ATTACHED,
    DFU_STATE_SET_INTERFACE,
    DFU_STATE_GET_STATUS,
    DFU_STATE_TRANSFER,
    DFU_STATE_ZERO_LENGTH_TRANSFER,
    DFU_STATE_READ_BACK,
    DFU_STATE_GET_STATUS_READ,
    DFU_STATE_DETACH,
    DFU_STATE_CHECK_STATUS,
    DFU_STATE_COMPLETE,
    DFU_STATE_ERROR,
};

enum class RequestType : uint8_t {
    NEXT_STATE,
};

struct NextStateRequest {
    dfu_state state;
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

static constexpr size_t kEdgeTpuDfuTaskStackDepth = configMINIMAL_STACK_SIZE * 3;
static constexpr UBaseType_t kEdgeTpuDfuTaskQueueLength = 4;
extern const char kEdgeTpuDfuTaskName[];

class EdgeTpuDfuTask : public QueueTask<edgetpu_dfu::Request, edgetpu_dfu::Response, kEdgeTpuDfuTaskName,
                                        kEdgeTpuDfuTaskStackDepth, EDGETPU_DFU_TASK_PRIORITY, kEdgeTpuDfuTaskQueueLength> {
  public:
    static EdgeTpuDfuTask* GetSingleton() {
        static EdgeTpuDfuTask task;
        return &task;
    }

    // USB event handler
    usb_status_t USB_DFUHostEvent(usb_host_handle host_handle, usb_device_handle device_handle,
                                  usb_host_configuration_handle config_handle,
                                  uint32_t event_code);

    // Callback methods for interacting with the USB stack
    static void SetInterfaceCallback(void *param, uint8_t *data,
                                     uint32_t data_length,
                                     usb_status_t status);
    static void GetStatusCallback(void *param, uint8_t *data,
                                  uint32_t data_length,
                                  usb_status_t status);
    static void TransferCallback(void *param,
                                 uint8_t *data,
                                 uint32_t data_length,
                                 usb_status_t status);
    static void ZeroLengthTransferCallback(void *param,
                                           uint8_t *data,
                                           uint32_t data_length,
                                           usb_status_t status);
    static void ReadBackCallback(void *param,
                                 uint8_t *data,
                                 uint32_t data_length,
                                 usb_status_t status);
    static void GetStatusReadCallback(void *param,
                                      uint8_t *data,
                                      uint32_t data_length,
                                      usb_status_t status);
    static void DetachCallback(void *param,
                               uint8_t *data,
                               uint32_t data_length,
                               usb_status_t status);
    static void CheckStatusCallback(void *param,
                                    uint8_t *data,
                                    uint32_t data_length,
                                    usb_status_t status);

    // Getters/setters for members
    void SetHostInstance(usb_host_instance_t* instance) {
      host_instance_ = instance;
    }
    usb_host_instance_t* host_instance() {
      return host_instance_;
    }

    void SetDeviceHandle(usb_device_handle handle) {
      device_handle_ = handle;
    }
    usb_device_handle device_handle() {
      return device_handle_;
    }

    void SetInterfaceHandle(usb_host_interface_handle handle) {
      interface_handle_ = handle;
    }
    usb_host_interface_handle interface_handle() {
      return interface_handle_;
    }

    void SetClassHandle(usb_host_class_handle handle) {
      class_handle_ = handle;
    }
    usb_host_class_handle class_handle() {
      return class_handle_;
    }

    void SetStatus(usb_host_dfu_status_t status) {
      status_ = status;
    }
    usb_host_dfu_status_t status() {
      return status_;
    }

    void SetBytesTransferred(size_t bytes) {
      bytes_transferred_ = bytes;
    }
    size_t bytes_transferred() {
      return bytes_transferred_;
    }

    void SetBytesToTransfer(size_t bytes) {
      bytes_to_transfer_ = bytes;
    }
    size_t bytes_to_transfer() {
      return bytes_to_transfer_;
    }

    void SetCurrentBlockNumber(size_t block) {
      current_block_number_ = block;
    }
    size_t current_block_number() {
      return current_block_number_;
    }

    void SetReadBackData(uint8_t *read_back_data) {
      read_back_data_ = read_back_data;
    }
    uint8_t* read_back_data() {
      return read_back_data_;
    }

  private:
    void TaskInit() override;
    void RequestHandler(edgetpu_dfu::Request *req) override;
    void HandleNextState(edgetpu_dfu::NextStateRequest& req);
    void SetNextState(enum edgetpu_dfu::dfu_state next_state);

    usb_host_instance_t* host_instance_;
    usb_device_handle device_handle_;
    usb_host_interface_handle interface_handle_;
    usb_host_class_handle class_handle_;
    usb_host_dfu_status_t status_;
    size_t bytes_transferred_ = 0;
    size_t bytes_to_transfer_ = apex_latest_single_ep_bin_len;
    size_t current_block_number_ = 0;
    uint8_t *read_back_data_ = nullptr;
};

}  // namespace valiant

#endif  // _APPS_LSUSB_DFU_H_
