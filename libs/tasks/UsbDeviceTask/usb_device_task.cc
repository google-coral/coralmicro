#include "libs/base/utils.h"
#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/clock_config.h"
#include "libs/nxp/rt1176-sdk/usb_device_config.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.h"

#include <cstdio>

constexpr int kUSBControllerId = kUSB_ControllerEhci0;

extern "C" void USB_OTG1_IRQHandler(void) {
    USB_DeviceEhciIsrFunction(coral::micro::UsbDeviceTask::GetSingleton()->device_handle());
}

// Super-basic unicode "conversion" function (no conversion really, just
// ascii into 2-byte expansion).
//
// Expects param->buffer to be set to a suitable output buffer
void ToUsbStringDescriptor(const char* s,
        usb_device_get_string_descriptor_struct_t* param) {
    uint8_t* output = param->buffer;

    // prefix is size and type
    *output++ = 2 + 2 * strlen(s);
    *output++ = USB_DESCRIPTOR_TYPE_STRING;
    param->length = 2;

    while (*s) {
        *output++ = *s++;
        *output++ = 0;
        param->length += 2;
    }
}


namespace coral::micro {

usb_status_t UsbDeviceTask::StaticHandler(usb_device_handle device_handle, uint32_t event, void *param) {
    return coral::micro::UsbDeviceTask::GetSingleton()->Handler(device_handle, event, param);
}

usb_status_t UsbDeviceTask::Handler(usb_device_handle device_handle, uint32_t event, void *param) {
    static uint8_t string_buffer[64];
    bool event_ret = true;
    usb_status_t ret = kStatus_USB_Error;
    usb_device_get_device_descriptor_struct_t *device_desc;
    usb_device_get_configuration_descriptor_struct_t *config_desc;
    usb_device_get_string_descriptor_struct_t *string_desc;
    switch (event) {
        case kUSB_DeviceEventBusReset:
            ret = kStatus_USB_Success;
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            device_desc = (usb_device_get_device_descriptor_struct_t*)param;
            device_desc->buffer = (uint8_t*)&device_descriptor_;
            device_desc->length = sizeof(device_descriptor_);
            ret = kStatus_USB_Success;
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            config_desc = (usb_device_get_configuration_descriptor_struct_t*)param;
            config_desc->buffer = composite_descriptor_.data();
            config_desc->length = composite_descriptor_.size();
            ret = kStatus_USB_Success;
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            string_desc = (usb_device_get_string_descriptor_struct_t*)param;
            string_desc->buffer = string_buffer;
            if (string_desc->stringIndex == 0) {
                string_desc->length = sizeof(lang_id_desc_);
                memcpy(string_desc->buffer, &lang_id_desc_, sizeof(lang_id_desc_));
                ret = kStatus_USB_Success;
                break;
            }
            if (string_desc->languageId != 0x0409) { // English
                ret = kStatus_USB_InvalidRequest;
                break;
            }
            switch (string_desc->stringIndex) {
                case 1:
                    ToUsbStringDescriptor("Google", string_desc);
                    ret = kStatus_USB_Success;
                    break;
                case 2:
                    ToUsbStringDescriptor("Valiant", string_desc);
                    ret = kStatus_USB_Success;
                    break;
                case 3:
                    ToUsbStringDescriptor(serial_number_.c_str(), string_desc);
                    ret = kStatus_USB_Success;
                    break;
                default:
                    printf("Unhandled string request: %d\r\n", string_desc->stringIndex);
                    ret = kStatus_USB_InvalidRequest;
                    break;
            }
            break;
        case kUSB_DeviceEventSetConfiguration:
            for (size_t i = 0; i < set_handle_callbacks_.size(); i++) {
                set_handle_callbacks_[i](configs_[i].classHandle);
            }
            for (size_t i = 0; i < handle_event_callbacks_.size(); i++) {
                event_ret &= handle_event_callbacks_[i](event, param);
            }
            if (event_ret) {
                ret = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventSetInterface:
        case kUSB_DeviceEventGetHidReportDescriptor:
            ret = kStatus_USB_Success;
            for (size_t i = 0; i < handle_event_callbacks_.size(); i++) {
                event_ret &= handle_event_callbacks_[i](event, param);
            }
            if (event_ret) {
                ret = kStatus_USB_Success;
            }
            break;
        default:
            printf("%s event unhandled 0x%lx\r\n", __func__, event);
    }
    return ret;
}

void UsbDeviceTask::AddDevice(usb_device_class_config_struct_t* config, usb_set_handle_callback sh_cb,
        usb_handle_event_callback he_cb, void *descriptor_data, size_t descriptor_data_size) {
    configs_.push_back(*config);
    set_handle_callbacks_.push_back(sh_cb);
    handle_event_callbacks_.push_back(he_cb);

    size_t old_size = composite_descriptor_size_;
    composite_descriptor_size_ += descriptor_data_size;
    memcpy(composite_descriptor_.data() + old_size, descriptor_data, descriptor_data_size);

    CompositeDescriptor *p_composite_descriptor = reinterpret_cast<CompositeDescriptor*>(composite_descriptor_.data());
    p_composite_descriptor->conf.total_length = composite_descriptor_size_;
}

bool UsbDeviceTask::Init() {
    usb_status_t ret;

    config_list_ = { configs_.data(), &UsbDeviceTask::StaticHandler, (uint8_t)configs_.size() };
    ret = USB_DeviceClassInit(kUSBControllerId, &config_list_, &device_handle_);
    if (ret != kStatus_USB_Success) {
        printf("USB_DeviceClassInit failed %d\r\n", ret);
        return false;
    }
    SDK_DelayAtLeastUs(50000, CLOCK_GetFreq(kCLOCK_CpuClk));
    ret = USB_DeviceRun(device_handle_);
    if (ret != kStatus_USB_Success) {
        printf("USB_DeviceRun failed\r\n");
        return false;
    }
    return true;
}

UsbDeviceTask::UsbDeviceTask() {
    serial_number_ = coral::micro::utils::GetSerialNumber();

    uint32_t usbClockFreq = 24000000;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
    CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    USB_EhciPhyInit(kUSBControllerId, BOARD_XTAL0_CLK_HZ, &phyConfig);

    IRQn_Type irqNumber = USB_OTG1_IRQn;
    NVIC_SetPriority(irqNumber, 6);
    NVIC_EnableIRQ(irqNumber);
    CompositeDescriptor composite_descriptor = {
        {
            sizeof(ConfigurationDescriptor),
            0x02,
            sizeof(CompositeDescriptor),
            0, // Managed by next_interface_value
            1,
            0,
            0x80, // kUsb11AndHigher
            250, // kUses500mA
        }, // ConfigurationDescriptor
    };
    composite_descriptor_size_ = sizeof(composite_descriptor);
    // composite_descriptor_.resize(sizeof(composite_descriptor));
    memcpy(composite_descriptor_.data(), &composite_descriptor, sizeof(composite_descriptor));
}

void UsbDeviceTask::UsbDeviceTaskFn() {
    USB_DeviceEhciTaskFunction(device_handle_);
}

}  // namespace coral::micro
