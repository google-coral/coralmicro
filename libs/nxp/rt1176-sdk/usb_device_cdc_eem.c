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

// clang-format off
#include "third_party/modified/nxp/rt1176-sdk/usb_device_config.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/include/usb.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_class.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.h"
// clang-format on

#if USB_DEVICE_CONFIG_CDC_EEM
#include "libs/nxp/rt1176-sdk/usb_device_cdc_eem.h"

USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(
    USB_DATA_ALIGN_SIZE) static usb_device_cdc_eem_struct_t
    g_cdcEemHandle[USB_DEVICE_CONFIG_CDC_EEM];

static usb_status_t USB_DeviceCdcEemAllocateHandle(
    usb_device_cdc_eem_struct_t **handle) {
  uint32_t count;
  for (count = 0; count < USB_DEVICE_CONFIG_CDC_EEM; ++count) {
    if (g_cdcEemHandle[count].handle == NULL) {
      *handle = &g_cdcEemHandle[count];
      return kStatus_USB_Success;
    }
  }
  return kStatus_USB_Busy;
}

static usb_status_t USB_DeviceCdcEemFreeHandle(
    usb_device_cdc_eem_struct_t *handle) {
  handle->handle = NULL;
  handle->configStruct = NULL;
  handle->configuration = 0;
  handle->alternate = 0;
  return kStatus_USB_Success;
}

static usb_status_t USB_DeviceCdcEemEndpointsDeinit(
    usb_device_cdc_eem_struct_t *cdcEemHandle) {
  usb_status_t error = kStatus_USB_Error;
  uint32_t count;

  if (cdcEemHandle->dataInterfaceHandle == NULL) {
    return error;
  }

  for (count = 0; count < cdcEemHandle->dataInterfaceHandle->endpointList.count;
       ++count) {
    error = USB_DeviceDeinitEndpoint(
        cdcEemHandle->handle,
        cdcEemHandle->dataInterfaceHandle->endpointList.endpoint[count]
            .endpointAddress);
  }
  cdcEemHandle->dataInterfaceHandle = NULL;

  return error;
}

static usb_status_t USB_DeviceCdcEemBulkIn(
    usb_device_handle handle,
    usb_device_endpoint_callback_message_struct_t *message,
    void *callbackParam) {
  usb_device_cdc_eem_struct_t *cdcEemHandle;
  usb_status_t status = kStatus_USB_Error;
  cdcEemHandle = (usb_device_cdc_eem_struct_t *)callbackParam;

  if (!cdcEemHandle) {
    return kStatus_USB_InvalidHandle;
  }
  cdcEemHandle->bulkIn.isBusy = 0;

  if (cdcEemHandle->configStruct && cdcEemHandle->configStruct->classCallback) {
    status = cdcEemHandle->configStruct->classCallback(
        (class_handle_t)cdcEemHandle, kUSB_DeviceEemEventSendResponse, message);
  }

  return status;
}

static usb_status_t USB_DeviceCdcEemBulkOut(
    usb_device_handle handle,
    usb_device_endpoint_callback_message_struct_t *message,
    void *callbackParam) {
  usb_device_cdc_eem_struct_t *cdcEemHandle;
  usb_status_t status = kStatus_USB_Error;
  cdcEemHandle = (usb_device_cdc_eem_struct_t *)callbackParam;

  if (!cdcEemHandle) {
    return kStatus_USB_InvalidHandle;
  }

  cdcEemHandle->bulkOut.isBusy = 0;

  if (cdcEemHandle->configStruct && cdcEemHandle->configStruct->classCallback) {
    status = cdcEemHandle->configStruct->classCallback(
        (class_handle_t)cdcEemHandle, kUSB_DeviceEemEventRecvResponse, message);
  }

  return status;
}

static usb_status_t USB_DeviceCdcEemEndpointsInit(
    usb_device_cdc_eem_struct_t *cdcEemHandle) {
  usb_device_interface_list_t *interfaceList;
  usb_device_interface_struct_t *interface = NULL;
  usb_status_t error = kStatus_USB_Error;
  uint32_t count;
  uint32_t index;

  if (cdcEemHandle == NULL) {
    return error;
  }

  if ((cdcEemHandle->configuration == 0) ||
      (cdcEemHandle->configuration >
       cdcEemHandle->configStruct->classInfomation->configurations)) {
    return error;
  }

  interfaceList = &cdcEemHandle->configStruct->classInfomation
                       ->interfaceList[cdcEemHandle->configuration - 1];
  for (count = 0; count < interfaceList->count; ++count) {
    if (interfaceList->interfaces[count].classCode ==
            USB_DEVICE_CONFIG_CDC_COMM_CLASS_CODE &&
        interfaceList->interfaces[count].subclassCode ==
            USB_DEVICE_CONFIG_CDC_EEM_SUBCLASS_CODE) {
      for (index = 0; index < interfaceList->interfaces[count].count; index++) {
        if (interfaceList->interfaces[count]
                .interface[index]
                .alternateSetting == cdcEemHandle->alternate) {
          interface = &interfaceList->interfaces[count].interface[index];
          break;
        }
      }
      cdcEemHandle->interfaceNumber =
          interfaceList->interfaces[count].interfaceNumber;
      break;
    }
  }
  if (interface == NULL) {
    return error;
  }
  cdcEemHandle->dataInterfaceHandle = interface;

  for (count = 0; count < interface->endpointList.count; ++count) {
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;
    epInitStruct.zlt = 0;
    epInitStruct.transferType =
        interface->endpointList.endpoint[count].interval;
    epInitStruct.endpointAddress =
        interface->endpointList.endpoint[count].endpointAddress;
    epInitStruct.maxPacketSize =
        interface->endpointList.endpoint[count].maxPacketSize;
    epInitStruct.transferType =
        interface->endpointList.endpoint[count].transferType;

    if ((((epInitStruct.endpointAddress &
           USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
          USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT) == USB_IN) &&
        (epInitStruct.transferType == USB_ENDPOINT_BULK)) {
      // TODO(atv): ACM fills in their pipe struct here
      cdcEemHandle->bulkIn.isBusy = 0;
      epCallback.callbackFn = USB_DeviceCdcEemBulkIn;
    } else if ((((epInitStruct.endpointAddress &
                  USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                 USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT) == USB_OUT) &&
               (epInitStruct.transferType == USB_ENDPOINT_BULK)) {
      // TODO(atv): ACM fills in their pipe struct here
      cdcEemHandle->bulkOut.isBusy = 0;
      epCallback.callbackFn = USB_DeviceCdcEemBulkOut;
    }
    epCallback.callbackParam = cdcEemHandle;
    error = USB_DeviceInitEndpoint(cdcEemHandle->handle, &epInitStruct,
                                   &epCallback);
  }

  return error;
}

usb_status_t USB_DeviceCdcEemInit(uint8_t controllerId,
                                  usb_device_class_config_struct_t *config,
                                  class_handle_t *handle) {
  usb_device_cdc_eem_struct_t *cdcEemHandle;
  usb_status_t error;

  error = USB_DeviceCdcEemAllocateHandle(&cdcEemHandle);
  if (error != kStatus_USB_Success) {
    return error;
  }

  error = USB_DeviceClassGetDeviceHandle(controllerId, &cdcEemHandle->handle);
  if (error != kStatus_USB_Success) {
    return error;
  }

  if (NULL == cdcEemHandle->handle) {
    return kStatus_USB_InvalidHandle;
  }

  cdcEemHandle->configStruct = config;
  cdcEemHandle->configuration = 0;
  cdcEemHandle->alternate = 0xFF;

  *handle = (class_handle_t)cdcEemHandle;
  return error;
}

usb_status_t USB_DeviceCdcEemDeinit(class_handle_t handle) {
  usb_device_cdc_eem_struct_t *cdcEemHandle;
  usb_status_t error;

  cdcEemHandle = (usb_device_cdc_eem_struct_t *)handle;
  if (cdcEemHandle == NULL) {
    return kStatus_USB_InvalidHandle;
  }

  error = USB_DeviceCdcEemEndpointsDeinit(cdcEemHandle);
  (void)USB_DeviceCdcEemFreeHandle(cdcEemHandle);

  return error;
}

usb_status_t USB_DeviceCdcEemEvent(void *handle, uint32_t event, void *param) {
  usb_device_cdc_eem_struct_t *cdcEemHandle;
  usb_device_class_event_t eventCode = (usb_device_class_event_t)event;
  usb_status_t error = kStatus_USB_Error;
  uint16_t interfaceAlternate;
  uint8_t alternate;
  uint8_t *temp8;

  usb_device_cdc_eem_request_param_struct_t reqParam;

  if (!param || !handle) {
    return kStatus_USB_InvalidHandle;
  }
  cdcEemHandle = (usb_device_cdc_eem_struct_t *)handle;

  switch (eventCode) {
    case kUSB_DeviceClassEventDeviceReset:
      cdcEemHandle->configuration = 0;
      break;
    case kUSB_DeviceClassEventSetConfiguration:
      temp8 = (uint8_t *)param;
      if (cdcEemHandle->configStruct == NULL) {
        break;
      }
      if (*temp8 == cdcEemHandle->configuration) {
        break;
      }
      error = USB_DeviceCdcEemEndpointsDeinit(cdcEemHandle);
      cdcEemHandle->configuration = *temp8;
      cdcEemHandle->alternate = 0;
      error = USB_DeviceCdcEemEndpointsInit(cdcEemHandle);
      if (error != kStatus_USB_Success) {
        DbgConsole_Printf("EemEndpointsInit failed\r\n");
      }
      break;
    case kUSB_DeviceClassEventClassRequest: {
      usb_device_control_request_struct_t *controlRequest =
          (usb_device_control_request_struct_t *)param;
      if ((controlRequest->setup->wIndex & 0xFF) !=
          cdcEemHandle->interfaceNumber) {
        break;
      }

      reqParam.buffer = &(controlRequest->buffer);
      reqParam.length = &(controlRequest->length);
      reqParam.interfaceIndex = controlRequest->setup->wIndex;
      reqParam.setupValue = controlRequest->setup->wValue;
      reqParam.isSetup = controlRequest->isSetup;

      if (controlRequest->setup->bRequest ==
          USB_DEVICE_CDC_REQUEST_SET_CONTROL_LINE_STATE) {
        error = cdcEemHandle->configStruct->classCallback(
            (class_handle_t)cdcEemHandle,
            kUSB_DeviceCdcEventSetControlLineState, &reqParam);
      } else {
        DbgConsole_Printf("[EEM] Unhandled class request %x %x request: %x\r\n",
                          controlRequest->setup->wIndex & 0xFF,
                          cdcEemHandle->interfaceNumber,
                          controlRequest->setup->bRequest);
      }
      break;
    }
    case kUSB_DeviceClassEventSetInterface:
      if (cdcEemHandle->configStruct == NULL) {
        break;
      }
      interfaceAlternate = *((uint16_t *)param);
      alternate = (uint8_t)(interfaceAlternate & 0xFF);

      if (cdcEemHandle->interfaceNumber !=
          ((uint8_t)(interfaceAlternate >> 8))) {
        break;
      }

      if (alternate == cdcEemHandle->alternate) {
        break;
      }
      error = USB_DeviceCdcEemEndpointsDeinit(cdcEemHandle);
      cdcEemHandle->alternate = alternate;
      error = USB_DeviceCdcEemEndpointsInit(cdcEemHandle);
      break;
    default:
      DbgConsole_Printf("%s unhandled %lu\r\n", __func__, event);
  }

  return error;
}

usb_status_t USB_DeviceCdcEemSend(class_handle_t handle, uint8_t ep,
                                  uint8_t *buffer, uint32_t length) {
  usb_device_cdc_eem_struct_t *cdcEemHandle;
  usb_status_t status = kStatus_USB_Error;
  usb_device_cdc_eem_pipe_t *cdcEemPipe;

  if (!handle) {
    return kStatus_USB_InvalidHandle;
  }
  cdcEemHandle = (usb_device_cdc_eem_struct_t *)handle;
  cdcEemPipe = &(cdcEemHandle->bulkIn);

  if (cdcEemPipe->isBusy) {
    return kStatus_USB_Busy;
  }
  cdcEemPipe->isBusy = 1;

  status = USB_DeviceSendRequest(cdcEemHandle->handle, ep, buffer, length);
  if (status != kStatus_USB_Success) {
    cdcEemPipe->isBusy = 0;
  }

  return status;
}

usb_status_t USB_DeviceCdcEemRecv(class_handle_t handle, uint8_t ep,
                                  uint8_t *buffer, uint32_t length) {
  usb_device_cdc_eem_struct_t *cdcEemHandle;
  usb_status_t status;

  if (!handle) {
    return kStatus_USB_InvalidHandle;
  }
  cdcEemHandle = (usb_device_cdc_eem_struct_t *)handle;

  if (cdcEemHandle->bulkOut.isBusy) {
    return kStatus_USB_Busy;
  }
  cdcEemHandle->bulkOut.isBusy = 1;

  status = USB_DeviceRecvRequest(cdcEemHandle->handle, ep, buffer, length);
  if (status != kStatus_USB_Success) {
    cdcEemHandle->bulkOut.isBusy = 0;
  }

  return status;
}

#endif  // USB_DEVICE_CONFIG_CDC_EEM
