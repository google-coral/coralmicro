#ifndef _USB_DEVICE_CDC_EEM_H_
#define _USB_DEVICE_CDC_EEM_H_

#ifndef USB_DEVICE_CONFIG_CDC_COMM_CLASS_CODE
#define USB_DEVICE_CONFIG_CDC_COMM_CLASS_CODE (0x02)
#endif

#define USB_DEVICE_CONFIG_CDC_EEM_SUBCLASS_CODE (0x0C)

#define EEM_HEADER_TYPE_MASK  (0x8000)
#define EEM_HEADER_TYPE_SHIFT (15)

#define EEM_DATA_CRC_MASK (0x4000)
#define EEM_DATA_CRC_SHIFT (14)
#define EEM_DATA_LEN_MASK (0x3FFF)
#define EEM_DATA_LEN_SHIFT (0)

#define EEM_COMMAND_OPCODE_MASK (0x3800)
#define EEM_COMMAND_OPCODE_SHIFT (11)
#define EEM_COMMAND_PARAM_MASK (0x07FF)
#define EEM_COMMAND_PARAM_SHIFT (0)

#define EEM_COMMAND_ECHO (0)
#define EEM_COMMAND_ECHO_RESPONSE (1)

typedef enum _usb_device_cdc_eem_event {
    kUSB_DeviceEemEventRecvResponse = 0x1,
    kUSB_DeviceEemEventSendResponse,
} usb_device_cdc_eem_event_t;

typedef struct _usb_device_cdc_eem_pipe {
    uint8_t isBusy;
} usb_device_cdc_eem_pipe_t;

typedef struct _usb_device_cdc_eem_struct {
    usb_device_handle handle;
    usb_device_class_config_struct_t *configStruct;
    usb_device_interface_struct_t *dataInterfaceHandle;
    usb_device_cdc_eem_pipe_t bulkIn;
    usb_device_cdc_eem_pipe_t bulkOut;
    uint8_t configuration;
    uint8_t alternate;
    uint8_t interfaceNumber;
} usb_device_cdc_eem_struct_t;

#if defined(__cplusplus)
extern "C" {
#endif

usb_status_t USB_DeviceCdcEemInit(
        uint8_t controllerId,
        usb_device_class_config_struct_t *config,
        class_handle_t *handle);

usb_status_t USB_DeviceCdcEemDeinit(class_handle_t handle);

usb_status_t USB_DeviceCdcEemEvent(
        void *handle,
        uint32_t event,
        void *param);

 usb_status_t USB_DeviceCdcEemSend(
         class_handle_t handle,
         uint8_t ep,
         uint8_t *buffer,
         uint32_t length);

usb_status_t USB_DeviceCdcEemRecv(
        class_handle_t handle,
        uint8_t ep,
        uint8_t *buffer,
        uint32_t length);

#if defined(__cplusplus)
}
#endif

#endif  // _USB_DEVICE_CDC_EEM_H_
