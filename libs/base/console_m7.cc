#include "libs/base/console_m7.h"

#include <unistd.h>

#include <cstdio>
#include <functional>

#include "libs/base/check.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/message_buffer.h"
#include "libs/base/mutex.h"
#include "libs/base/tasks.h"
#include "libs/usb/usb_device_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

using namespace std::placeholders;

extern "C" int DbgConsole_SendDataReliable(uint8_t*, size_t);
extern "C" int _write(int handle, char* buffer, int size) {
    if ((handle != STDOUT_FILENO) && (handle != STDERR_FILENO)) {
        return -1;
    }

    coral::micro::ConsoleM7::GetSingleton()->Write(buffer, size);

    return size;
}

extern "C" int _read(int handle, char* buffer, int size) {
    if (handle != STDIN_FILENO) {
        return -1;
    }

    int bytes_read =
        coral::micro::ConsoleM7::GetSingleton()->Read(buffer, size);
    return bytes_read > 0 ? bytes_read : -1;
}

namespace coral::micro {

uint8_t ConsoleM7::m4_console_buffer_storage_[kM4ConsoleBufferSize]
    __attribute__((section(".noinit.$rpmsg_sh_mem")));

void ConsoleM7::Write(char* buffer, int size) {
    if (!tx_task_) {
        return;
    }
    ConsoleMessage msg = {
        size,
        (uint8_t*)malloc(size),
    };
#ifdef BLOCKING_PRINTF
    msg.semaphore = xSemaphoreCreateBinaryStatic(&msg.semaphore_storage);
#endif
    memcpy(msg.str, buffer, size);
    xQueueSend(console_queue_, &msg, portMAX_DELAY);
#ifdef BLOCKING_PRINTF
    xSemaphoreTake(msg.semaphore, portMAX_DELAY);
    vSemaphoreDelete(msg.semaphore);
#endif
}

int ConsoleM7::Read(char* buffer, int size) {
    if (!rx_task_) {
        return -1;
    }
    MutexLock lock(rx_mutex_);
    int bytes_to_return =
        std::min(size, static_cast<int>(rx_buffer_available_));

    if (!bytes_to_return) {
        return -1;
    }

    int bytes_to_read = bytes_to_return;
    if (rx_buffer_read_ > rx_buffer_write_) {
        memcpy(buffer, &rx_buffer_[rx_buffer_read_],
               kRxBufferSize - rx_buffer_read_);
        bytes_to_read -= kRxBufferSize - rx_buffer_read_;
        if (bytes_to_read) {
            memcpy(buffer + (bytes_to_return - bytes_to_read), &rx_buffer_[0],
                   bytes_to_read);
        }
    } else {
        memcpy(buffer, &rx_buffer_[rx_buffer_read_], bytes_to_read);
    }
    rx_buffer_available_ -= bytes_to_return;
    rx_buffer_read_ = (rx_buffer_read_ + bytes_to_return) % kRxBufferSize;

    return bytes_to_return;
}

void ConsoleM7::M4ConsoleTaskFn(void* param) {
    ipc::Message m4_console_buffer_msg;
    m4_console_buffer_msg.type = ipc::MessageType::kSystem;
    m4_console_buffer_msg.message.system.type =
        ipc::SystemMessageType::CONSOLE_BUFFER_PTR;
    m4_console_buffer_msg.message.system.message.console_buffer_ptr =
        GetM4ConsoleBufferPtr();
    IPCM7::GetSingleton()->SendMessage(m4_console_buffer_msg);

    size_t rx_bytes;
    char buf[16];
    while (true) {
        rx_bytes = xStreamBufferReceive(m4_console_buffer_->stream_buffer, buf,
                                        sizeof(buf), pdMS_TO_TICKS(10));
        if (rx_bytes > 0) {
            fwrite(buf, 1, rx_bytes, stdout);
        }
    }
}

// TODO(atv): At the moment, this only reads from DbgConsole, not USB.
void ConsoleM7::M7ConsoleTaskRxFn(void* param) {
    while (true) {
        uint8_t ch = static_cast<uint8_t>(DbgConsole_Getchar());
        MutexLock lock(rx_mutex_);
        assert(rx_buffer_write_ < kRxBufferSize);
        rx_buffer_[rx_buffer_write_] = ch;
        rx_buffer_write_ = (rx_buffer_write_ + 1) % kRxBufferSize;
        if (rx_buffer_write_ == rx_buffer_read_) {
            rx_buffer_read_ = (rx_buffer_read_ + 1) % kRxBufferSize;
            assert(rx_buffer_read_ < kRxBufferSize);
        } else {
            ++rx_buffer_available_;
            if (rx_buffer_available_ > kRxBufferSize) {
            }
            assert(rx_buffer_available_ <= kRxBufferSize);
        }
    }
}

void ConsoleM7::M7ConsoleTaskTxFn(void* param) {
    while (true) {
        ConsoleMessage msg;
        if (xQueueReceive(console_queue_, &msg, portMAX_DELAY) == pdTRUE) {
            DbgConsole_SendDataReliable((uint8_t*)msg.str, msg.len);
            cdc_acm_.Transmit((uint8_t*)msg.str, msg.len);
            free(msg.str);
#ifdef BLOCKING_PRINTF
            DbgConsole_Flush();
            xSemaphoreGive(msg.semaphore);
#endif
        }
    }
}

void usb_device_task(void* param) {
    while (true) {
        coral::micro::UsbDeviceTask::GetSingleton()->UsbDeviceTaskFn();
        taskYIELD();
    }
}

void ConsoleM7::Init(bool init_tx, bool init_rx) {
    m4_console_buffer_ =
        reinterpret_cast<ipc::StreamBuffer*>(m4_console_buffer_storage_);
    m4_console_buffer_->stream_buffer = xStreamBufferCreateStatic(
        kM4ConsoleBufferBytes, 1, m4_console_buffer_->stream_buffer_storage,
        &m4_console_buffer_->static_stream_buffer);

    cdc_acm_.Init(
        coral::micro::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
        coral::micro::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
        coral::micro::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
        coral::micro::UsbDeviceTask::GetSingleton()->next_interface_value(),
        coral::micro::UsbDeviceTask::GetSingleton()->next_interface_value(),
        nullptr /*ReceiveHandler*/);
    coral::micro::UsbDeviceTask::GetSingleton()->AddDevice(
        cdc_acm_.config_data(),
        std::bind(&coral::micro::CdcAcm::SetClassHandle, &cdc_acm_, _1),
        std::bind(&coral::micro::CdcAcm::HandleEvent, &cdc_acm_, _1, _2),
        cdc_acm_.descriptor_data(), cdc_acm_.descriptor_data_size());

    console_queue_ = xQueueCreate(16, sizeof(ConsoleMessage));
    CHECK(console_queue_);

    rx_mutex_ = xSemaphoreCreateMutex();
    CHECK(rx_mutex_);

    CHECK(xTaskCreate(usb_device_task, "usb_device_task",
                      configMINIMAL_STACK_SIZE * 10, nullptr,
                      USB_DEVICE_TASK_PRIORITY, nullptr) == pdPASS);
    if (init_tx) {
        CHECK(xTaskCreate(StaticM7ConsoleTaskTxFn, "m7_console_task_tx",
                          configMINIMAL_STACK_SIZE * 10, nullptr,
                          CONSOLE_TASK_PRIORITY, &tx_task_) == pdPASS);
    }
    if (init_rx) {
        CHECK(xTaskCreate(StaticM7ConsoleTaskRxFn, "m7_console_task_rx",
                          configMINIMAL_STACK_SIZE * 10, nullptr,
                          CONSOLE_TASK_PRIORITY, &rx_task_) == pdPASS);
    }
    if (IPCM7::HasM4Application()) {
        CHECK(xTaskCreate(StaticM4ConsoleTaskFn, "m4_console_task",
                          configMINIMAL_STACK_SIZE * 10, nullptr,
                          CONSOLE_TASK_PRIORITY, nullptr) == pdPASS);
    }
}

ipc::StreamBuffer* ConsoleM7::GetM4ConsoleBufferPtr() {
    return m4_console_buffer_;
}

}  // namespace coral::micro
