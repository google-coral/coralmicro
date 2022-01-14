#include "apps/ELFLoader/elf_loader.h"
#include "libs/base/filesystem.h"
#include "libs/base/reset.h"
#include "libs/base/tasks.h"
#include "libs/nxp/rt1176-sdk/board_hardware.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "libs/usb/descriptors.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_soc_src.h"
#include <elf.h>
#include <memory>

/* Function definitions */
static void elfloader_main(void *param);
static void elfloader_recv(const uint8_t *buffer, uint32_t length);
static void usb_timer_callback(TimerHandle_t timer);
static bool elfloader_HandleEvent(uint32_t event, void *param);
static void elfloader_SetClassHandle(class_handle_t class_handle);
static usb_status_t elfloader_Handler(class_handle_t class_handle, uint32_t event, void *param);
void usb_device_task(void *param);

extern "C" uint32_t vPortGetRunTimeCounterValue(void) {
    return 0;
}

/* Static data definitions */
TimerHandle_t usb_timer;
static uint8_t* elfloader_recv_image = nullptr;
static char* elfloader_recv_path = nullptr;
static uint8_t elfloader_data[64];
static class_handle_t elfloader_class_handle;
static ElfloaderTarget elfloader_target = ElfloaderTarget::Ram;
static lfs_file_t file_handle;

static void elfloader_recv(const uint8_t *buffer, uint32_t length) {
    ElfloaderCommand cmd = static_cast<ElfloaderCommand>(buffer[0]);
    const ElfloaderSetSize *set_size = reinterpret_cast<const ElfloaderSetSize*>(&buffer[1]);
    const ElfloaderBytes *bytes = reinterpret_cast<const ElfloaderBytes*>(&buffer[1]);
    const ElfloaderTarget *target = reinterpret_cast<const ElfloaderTarget*>(&buffer[1]);
    xTimerStop(usb_timer, 0);
    switch (cmd) {
        case ElfloaderCommand::SetSize:
            assert(length == sizeof(ElfloaderSetSize) + 1);
            switch (elfloader_target) {
                case ElfloaderTarget::Filesystem:
                    break;
                case ElfloaderTarget::Ram:
                    elfloader_recv_image = (uint8_t*)malloc(set_size->size);
                    break;
                case ElfloaderTarget::Path:
                    elfloader_recv_path = (char*)malloc(set_size->size + 1);
                    memset(elfloader_recv_path, 0, set_size->size + 1);
                    break;
            }
            break;
        case ElfloaderCommand::Bytes:
            assert(length >= sizeof(ElfloaderBytes) + 1);
            switch (elfloader_target) {
                case ElfloaderTarget::Ram:
                    memcpy(elfloader_recv_image + bytes->offset, &buffer[1] + sizeof(ElfloaderBytes), bytes->size);
                    break;
                case ElfloaderTarget::Path:
                    memcpy(elfloader_recv_path + bytes->offset, &buffer[1] + sizeof(ElfloaderBytes), bytes->size);
                    break;
                case ElfloaderTarget::Filesystem:
                    valiant::filesystem::Write(&file_handle, &buffer[1] + sizeof(ElfloaderBytes), bytes->size);
                    break;
            }
            break;
        case ElfloaderCommand::Done:
            switch (elfloader_target) {
                case ElfloaderTarget::Ram:
                    xTaskCreate(elfloader_main, "elfloader_main", configMINIMAL_STACK_SIZE * 10, elfloader_recv_image, APP_TASK_PRIORITY, NULL);
                    break;
                case ElfloaderTarget::Path: {
                        // TODO(atv): This stuff can fail. We should propagate errors back to the Python side if possible.
                        auto dir = valiant::filesystem::Dirname(elfloader_recv_path);
                        valiant::filesystem::MakeDirs(dir.get());
                        valiant::filesystem::Open(&file_handle, elfloader_recv_path, true);
                        free(elfloader_recv_path);
                        elfloader_recv_path = nullptr;
                    }
                    break;
                case ElfloaderTarget::Filesystem:
                    valiant::filesystem::Close(&file_handle);
                    break;
            }
            break;
        case ElfloaderCommand::Reset:
            valiant::ResetToBootloader();
            break;
        case ElfloaderCommand::Target:
            elfloader_target = *target;
            break;
    }
}

static bool elfloader_HandleEvent(uint32_t event, void *param) {
    bool ret = true;
    usb_device_get_hid_descriptor_struct_t* get_hid_descriptor =
        static_cast<usb_device_get_hid_descriptor_struct_t*>(param);
    switch (event) {
        case kUSB_DeviceEventSetConfiguration:
            USB_DeviceHidRecv(elfloader_class_handle, elfloader_hid_endpoints[kRxEndpoint].endpointAddress, elfloader_data, sizeof(elfloader_data));
            break;
        case kUSB_DeviceEventGetHidReportDescriptor:
            get_hid_descriptor->buffer = elfloader_hid_report;
            get_hid_descriptor->length = elfloader_hid_report_size;
            get_hid_descriptor->interfaceNumber = elfloader_interfaces[0].interfaceNumber;
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

static void elfloader_SetClassHandle(class_handle_t class_handle) {
    elfloader_class_handle = class_handle;
}

static usb_status_t elfloader_Handler(class_handle_t class_handle, uint32_t event, void *param) {
    uint8_t dummy = 0;
    usb_status_t ret = kStatus_USB_Success;
    usb_device_endpoint_callback_message_struct_t *message =
        static_cast<usb_device_endpoint_callback_message_struct_t*>(param);
    switch (event) {
        case kUSB_DeviceHidEventRecvResponse:
            if (message->length != USB_UNINITIALIZED_VAL_32) {
                elfloader_recv(message->buffer, message->length);
            }
            USB_DeviceHidSend(elfloader_class_handle, elfloader_hid_endpoints[kTxEndpoint].endpointAddress, &dummy, 1);
            USB_DeviceHidRecv(elfloader_class_handle, elfloader_hid_endpoints[kRxEndpoint].endpointAddress, elfloader_data, sizeof(elfloader_data));
            break;
        case kUSB_DeviceHidEventGetReport:
            ret = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventSendResponse:
        case kUSB_DeviceHidEventSetIdle:
            break;
        default:
            ret = kStatus_USB_Error;
            break;
    }
    return ret;
}

static usb_device_class_config_struct_t elfloader_config_data_ = {
    elfloader_Handler, nullptr, &elfloader_class_struct,
};

typedef void (*entry_point)(void);
static void elfloader_main(void *param) {
    size_t elf_size;
    std::unique_ptr<uint8_t> application_elf;
    if (!param) {
        application_elf.reset(valiant::filesystem::ReadToMemory("/default.elf", &elf_size));
    } else {
        application_elf.reset(reinterpret_cast<uint8_t*>(param));
    }

    Elf32_Ehdr* elf_header = reinterpret_cast<Elf32_Ehdr*>(application_elf.get());
    assert(EF_ARM_EABI_VERSION(elf_header->e_flags) == EF_ARM_EABI_VER5);
    assert(elf_header->e_phentsize == sizeof(Elf32_Phdr));

    for (int i = 0; i < elf_header->e_phnum; ++i) {
        Elf32_Phdr *program_header = reinterpret_cast<Elf32_Phdr*>(application_elf.get() + elf_header->e_phoff + sizeof(Elf32_Phdr) * i);
        if (program_header->p_type != PT_LOAD) {
            continue;
        }
        if (program_header->p_filesz == 0) {
            continue;
        }
        memcpy(reinterpret_cast<void*>(program_header->p_paddr), application_elf.get() + program_header->p_offset, program_header->p_filesz);
    }

    entry_point entry_point_fn = reinterpret_cast<entry_point>(elf_header->e_entry);
    entry_point_fn();

    vTaskSuspend(NULL);
}

void usb_device_task(void *param) {
    while (true) {
        valiant::UsbDeviceTask::GetSingleton()->UsbDeviceTaskFn();
        taskYIELD();
    }
}

static void usb_timer_callback(TimerHandle_t timer) {
    vTaskSuspend(reinterpret_cast<TaskHandle_t>(pvTimerGetTimerID(timer)));
    xTaskCreate(elfloader_main, "elfloader_main", configMINIMAL_STACK_SIZE * 10, nullptr, APP_TASK_PRIORITY, NULL);
}

extern "C" int main(int argc, char **argv) {
    BOARD_InitHardware(false);
    valiant::filesystem::Init();

    TaskHandle_t usb_task;
    xTaskCreate(usb_device_task, "usb_device_task", configMINIMAL_STACK_SIZE * 10, NULL, USB_DEVICE_TASK_PRIORITY, &usb_task);
    usb_timer = xTimerCreate("usb_timer", pdMS_TO_TICKS(1000), pdFALSE, usb_task, usb_timer_callback);

    // See TRM chapter 10.3.1 for boot mode choices.
    // If boot mode is *not* the serial downloader, start the boot timer.
    if (SRC_GetBootMode(SRC) != kSerialDownloader) {
        xTimerStart(usb_timer, 0);
    }

    elfloader_hid_endpoints[0].endpointAddress =
        valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value() | (USB_IN << 7);
    elfloader_hid_endpoints[1].endpointAddress =
        valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value() | (USB_OUT << 7);
    elfloader_descriptor_data.in_ep.endpoint_address = elfloader_hid_endpoints[0].endpointAddress;
    elfloader_descriptor_data.out_ep.endpoint_address = elfloader_hid_endpoints[1].endpointAddress;
    elfloader_interfaces[0].interfaceNumber =
        valiant::UsbDeviceTask::GetSingleton()->next_interface_value();
    valiant::UsbDeviceTask::GetSingleton()->AddDevice(
        &elfloader_config_data_,
        elfloader_SetClassHandle,
        elfloader_HandleEvent,
        &elfloader_descriptor_data,
        sizeof(elfloader_descriptor_data)
    );

    valiant::UsbDeviceTask::GetSingleton()->Init();

    vTaskStartScheduler();
    return 0;
}
