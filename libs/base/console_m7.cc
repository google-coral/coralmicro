#include "libs/base/console_m7.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/message_buffer.h"
#include "libs/base/tasks.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include <cstdio>
#include <functional>
#include <unistd.h>

using namespace std::placeholders;

extern "C" int DbgConsole_SendDataReliable(uint8_t*, size_t);
extern "C" int _write(int handle, char *buffer, int size) {
    if ((handle != STDOUT_FILENO) && (handle != STDERR_FILENO)) {
        return -1;
    }

    valiant::ConsoleM7::GetSingleton()->Write(buffer, size);

    return size;
}

extern "C" int _read(int handle, char *buffer, int size) {
    if (handle != STDIN_FILENO) {
        return -1;
    }

    return -1;
}

namespace valiant {

uint8_t ConsoleM7::m4_console_buffer_storage_[kM4ConsoleBufferSize] __attribute__((section(".noinit.$rpmsg_sh_mem")));

void ConsoleM7::Write(char *buffer, int size) {
    ConsoleMessage msg = {
        size, (uint8_t*)malloc(size),
    };
    memcpy(msg.str, buffer, size);
    xQueueSend(console_queue_, &msg, portMAX_DELAY);
}

void ConsoleM7::StaticM4ConsoleTaskFn(void *param) {
    GetSingleton()->M4ConsoleTaskFn(param);
}

void ConsoleM7::M4ConsoleTaskFn(void *param) {
    ipc::Message m4_console_buffer_msg;
    m4_console_buffer_msg.type = ipc::MessageType::SYSTEM;
    m4_console_buffer_msg.message.system.type = ipc::SystemMessageType::CONSOLE_BUFFER_PTR;
    m4_console_buffer_msg.message.system.message.console_buffer_ptr = GetM4ConsoleBufferPtr();
    IPC::GetSingleton()->SendMessage(m4_console_buffer_msg);

    size_t rx_bytes;
    char buf[16];
    while (true) {
        rx_bytes = xStreamBufferReceive(m4_console_buffer_->stream_buffer, buf, sizeof(buf), pdMS_TO_TICKS(10));
        if (rx_bytes > 0) {
            fwrite(buf, 1, rx_bytes, stdout);
        }
    }
}

void ConsoleM7::StaticM7ConsoleTaskFn(void *param) {
    GetSingleton()->M7ConsoleTaskFn(param);
}

void ConsoleM7::M7ConsoleTaskFn(void *param) {
    while (true) {
        ConsoleMessage msg;
        if (xQueueReceive(console_queue_, &msg, portMAX_DELAY) == pdTRUE) {
            DbgConsole_SendDataReliable((uint8_t*)msg.str, msg.len);
            cdc_acm_.Transmit((uint8_t*)msg.str, msg.len);
            free(msg.str);
        }
    }
}

void usb_device_task(void *param) {
    while (true) {
        valiant::UsbDeviceTask::GetSingleton()->UsbDeviceTaskFn();
        taskYIELD();
    }
}

void ConsoleM7::Init() {
    m4_console_buffer_ = reinterpret_cast<ipc::StreamBuffer*>(m4_console_buffer_storage_);
    m4_console_buffer_->stream_buffer =
        xStreamBufferCreateStatic(kM4ConsoleBufferBytes, 1, m4_console_buffer_->stream_buffer_storage, &m4_console_buffer_->static_stream_buffer);

    cdc_acm_.Init(
            valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_interface_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_interface_value(),
            nullptr /*ReceiveHandler*/);
    valiant::UsbDeviceTask::GetSingleton()->AddDevice(cdc_acm_.config_data(),
            std::bind(&valiant::CdcAcm::SetClassHandle, &cdc_acm_, _1),
            std::bind(&valiant::CdcAcm::HandleEvent, &cdc_acm_, _1, _2),
            cdc_acm_.descriptor_data(), cdc_acm_.descriptor_data_size());

    console_queue_ = xQueueCreate(16, sizeof(ConsoleMessage));
    if (!console_queue_) {
        DbgConsole_Printf("Failed to create console_queue\r\n");
        vTaskSuspend(NULL);
    }

    xTaskCreate(usb_device_task, "usb_device_task", configMINIMAL_STACK_SIZE * 10, NULL, USB_DEVICE_TASK_PRIORITY, NULL);
    xTaskCreate(StaticM7ConsoleTaskFn, "m7_console_task", configMINIMAL_STACK_SIZE * 10, NULL, CONSOLE_TASK_PRIORITY, NULL);
    if (IPCM7::HasM4Application()) {
        xTaskCreate(StaticM4ConsoleTaskFn, "m4_console_task", configMINIMAL_STACK_SIZE * 10, NULL, CONSOLE_TASK_PRIORITY, NULL);
    }
}

ipc::StreamBuffer* ConsoleM7::GetM4ConsoleBufferPtr() {
    return m4_console_buffer_;
}

}  // namespace valiant
