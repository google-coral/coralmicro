#include "libs/base/gpio.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/UsbHostTask/usb_host_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/usb_host_edgetpu/usb_host_edgetpu.h"

#include <cstdio>
#include <functional>

using namespace std::placeholders;

namespace coral::micro {
using namespace edgetpu;

void EdgeTpuTask::SetNextState(enum edgetpu_state next_state) {
    Request req;
    req.type = RequestType::NEXT_STATE;
    req.request.next_state.state = next_state;
    SendRequestAsync(req);
}

usb_status_t EdgeTpuTask::USBHostEvent(
            usb_host_handle host_handle,
            usb_device_handle device_handle,
            usb_host_configuration_handle config_handle,
            uint32_t event_code) {
    usb_host_configuration_t *configuration_ptr;
    switch (event_code & 0xFFFF) {
        case kUSB_HostEventAttach:
            configuration_ptr = reinterpret_cast<usb_host_configuration_t*>(config_handle);
            for (unsigned int i = 0; i < configuration_ptr->interfaceCount; ++i) {
                auto* interface_ptr = &configuration_ptr->interfaceList[i];
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
            return (this->device_handle() != nullptr) ? kStatus_USB_Success : kStatus_USB_NotSupported;
        case kUSB_HostEventEnumerationDone:
            SetNextState(EDGETPU_STATE_ATTACHED);
            return kStatus_USB_Success;
        case kUSB_HostEventDetach:
#if 0
            printf("EdgeTPU went away...\r\n");
#endif
            SetNextState(EDGETPU_STATE_UNATTACHED);
            USB_HostEdgeTpuDeinit(this->device_handle(), this->class_handle());
            return USB_HostRemoveDevice(host_handle, this->device_handle());
        default:
            return kStatus_USB_Success;
    }
}

void EdgeTpuTask::SetInterfaceCallback(void *param, uint8_t *data, uint32_t data_length, usb_status_t status) {
    auto *task = static_cast<EdgeTpuTask*>(param);
    if (status != kStatus_USB_Success) {
        printf("Error in EdgeTpuSetInterface\r\n");
        task->SetNextState(EDGETPU_STATE_ERROR);
        return;
    }
    task->SetNextState(EDGETPU_STATE_GET_STATUS);
}

void EdgeTpuTask::GetStatusCallback(void *param, uint8_t *data, uint32_t data_length, usb_status_t status) {
    auto *task = static_cast<EdgeTpuTask*>(param);
    if (status != kStatus_USB_Success) {
        printf("Error in EdgeTpuGetStatus\r\n");
        task->SetNextState(EDGETPU_STATE_ERROR);
        return;
    }
    task->SetNextState(EDGETPU_STATE_CONNECTED);
}

void EdgeTpuTask::TaskInit() {
    coral::micro::UsbHostTask::GetSingleton()->RegisterUSBHostEventCallback(
            kEdgeTpuVid, kEdgeTpuPid, std::bind(&EdgeTpuTask::USBHostEvent, this, _1, _2, _3, _4));
}

bool EdgeTpuTask::HandleGetPowerRequest() {
    return (enabled_count_ > 0);
}

void EdgeTpuTask::HandleSetPowerRequest(SetPowerRequest& req) {
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
#if defined(BOARD_REVISION_P0) || defined(BOARD_REVISION_P1)
    gpio::SetGpio(gpio::Gpio::kEdgeTpuPmic, req.enable);

    if (req.enable) {
        bool pgood;
        do {
            pgood = gpio::GetGpio(gpio::Gpio::kEdgeTpuPgood);
            taskYIELD();
        } while (!pgood);

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    gpio::SetGpio(gpio::Gpio::kEdgeTpuReset, req.enable);
#endif
}

void EdgeTpuTask::HandleNextState(NextStateRequest& req) {
    usb_status_t ret;
    enum edgetpu_state next_state = req.state;

    switch (next_state) {
        case EDGETPU_STATE_UNATTACHED:
            EdgeTpuManager::GetSingleton()->NotifyConnected(nullptr);
            break;
        case EDGETPU_STATE_ATTACHED:
            ret = USB_HostEdgeTpuInit(device_handle(), &class_handle_);
            if (ret != kStatus_USB_Success) {
                SetNextState(EDGETPU_STATE_ERROR);
            } else {
                SetNextState(EDGETPU_STATE_SET_INTERFACE);
            }
            break;
        case EDGETPU_STATE_SET_INTERFACE:
            ret = USB_HostEdgeTpuSetInterface(
                    class_handle(), interface_handle(),
                    0, SetInterfaceCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(EDGETPU_STATE_ERROR);
            }
            // Thunderchild notifies EdgeTpuManager that it's connected. Should this be in callback?
            EdgeTpuManager::GetSingleton()->NotifyConnected(reinterpret_cast<usb_host_edgetpu_instance_t*>(class_handle()));
            break;
        case EDGETPU_STATE_GET_STATUS:
            ret = USB_HostEdgeTpuGetStatus(class_handle(), &status_, GetStatusCallback, this);
            if (ret != kStatus_USB_Success) {
                SetNextState(EDGETPU_STATE_ERROR);
            }
            break;
        // case EDGETPU_STATE_CONNECTED:
        //     EdgeTpuManager::GetSingleton()->NotifyConnected(reinterpret_cast<usb_host_edgetpu_instance_t*>(class_handle()));
        //     break;
        case EDGETPU_STATE_ERROR:
            printf("EdgeTPU error\r\n");
            while (true) {
                taskYIELD();
            }
            break;
        default:
            printf("Unhandled EdgeTPU state: %d\r\n", next_state);
            while (true) {
                taskYIELD();
            }
            break;
    }
}

bool EdgeTpuTask::GetPower() {
    Request req;
    req.type = RequestType::GET_POWER;
    Response resp = SendRequest(req);
    return resp.response.get_power.enabled;
}

void EdgeTpuTask::SetPower(bool enable) {
    Request req;
    req.type = RequestType::SET_POWER;
    req.request.set_power.enable = enable;
    SendRequest(req);
}

void EdgeTpuTask::RequestHandler(Request *req) {
    Response resp;
    resp.type = req->type;
    switch (req->type) {
        case RequestType::NEXT_STATE:
            HandleNextState(req->request.next_state);
            break;
        case RequestType::SET_POWER:
            HandleSetPowerRequest(req->request.set_power);
            break;
        case RequestType::GET_POWER:
            resp.response.get_power.enabled = HandleGetPowerRequest();
            break;
    }

    if (req->callback) {
        req->callback(resp);
    }
}

}  // namespace coral::micro
