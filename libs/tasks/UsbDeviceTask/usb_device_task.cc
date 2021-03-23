#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/clock_config.h"
#include "libs/nxp/rt1176-sdk/usb_device_config.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.h"

#include <cstdio>
#include <functional>

using namespace std::placeholders;
constexpr int kUSBControllerId = kUSB_ControllerEhci0;

extern "C" void USB_OTG1_IRQHandler(void) {
    USB_DeviceEhciIsrFunction(valiant::UsbDeviceTask::GetSingleton()->device_handle());
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


namespace valiant {

usb_status_t UsbDeviceTask::StaticHandler(usb_device_handle device_handle, uint32_t event, void *param) {
    return valiant::UsbDeviceTask::GetSingleton()->Handler(device_handle, event, param);
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
            config_desc->buffer = (uint8_t*)&composite_descriptor_;
            config_desc->length = sizeof(composite_descriptor_);
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
                    ToUsbStringDescriptor("DEADBEEF", string_desc);
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
        usb_handle_event_callback he_cb) {
    configs_.push_back(*config);
    set_handle_callbacks_.push_back(sh_cb);
    handle_event_callbacks_.push_back(he_cb);
}

bool UsbDeviceTask::Init() {
    usb_status_t ret;

    cdc_eem_.Init(
            next_descriptor_value(),
            next_descriptor_value(),
            next_interface_value());
    AddDevice(cdc_eem_.config_data(),
            std::bind(&valiant::CdcEem::SetClassHandle, &cdc_eem_, _1),
            std::bind(&valiant::CdcEem::HandleEvent, &cdc_eem_, _1, _2));

    config_list_ = { configs_.data(), &UsbDeviceTask::StaticHandler, (uint8_t)configs_.size() };
    ret = USB_DeviceClassInit(kUSBControllerId, &config_list_, &device_handle_);
    if (ret != kStatus_USB_Success) {
        printf("USB_DeviceClassInit failed %d\r\n", ret);
        return false;
    }
    ret = USB_DeviceRun(device_handle_);
    if (ret != kStatus_USB_Success) {
        printf("USB_DeviceRun failed\r\n");
        return false;
    }
    return true;
}

UsbDeviceTask::UsbDeviceTask() {
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
}

void UsbDeviceTask::UsbDeviceTaskFn() {
    USB_DeviceEhciTaskFunction(device_handle_);
}

}  // namespace valiant
