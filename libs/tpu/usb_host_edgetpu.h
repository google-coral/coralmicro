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

#ifndef LIBS_TPU_USB_HOST_EDGETPU_H_
#define LIBS_TPU_USB_HOST_EDGETPU_H_

#include "usb.h"
#include "usb_host_config.h" // Must be above "usb_host.h"
#include "usb_host.h"
#include "usb_spec.h"

#define USB_HOST_EDGETPU_CLASS_CODE (0xFF)
#define USB_HOST_EDGETPU_SUBCLASS_CODE (0xFF)
#define USB_EDGETPU_ENDPOINT_NUM 6
#define USB_EDGETPU_BULK_OUT_ENDPOINT_NUM 3
#define USB_EDGETPU_BULK_IN_ENDPOINT_NUM 2
#define USB_EDGETPU_INTERRUPT_IN_ENDPOINT_NUM 1
#define USB_EDGETPU_BULK_OUT_PACKET_SIZE 512
#define USB_EDGETPU_BULK_IN_PACKET_SIZE 256
#define USB_EDGETPU_INTERRRUPT_ENDPOINT_INDEX 5

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  USB_EDGETPU_TRANSFER_READY,
  USB_EDGETPU_TRANSFER_BUSY,
} usb_host_edgetpu_transfer_status_t;

typedef struct _usb_host_edgetpu_pipe {
  usb_host_pipe_handle pipeHandle;
  uint8_t pipeType;
  uint16_t packetSize;
  uint8_t endPoint;
  uint8_t direction;
  transfer_callback_t callbackFn;
  void *callbackParam;
  usb_host_transfer_t *activeTransfer;
  usb_host_edgetpu_transfer_status_t transferStatus;
  bool connected;
} usb_host_edgetpu_pipe_t;

typedef struct _usb_host_edgetpu_instance {
  usb_host_handle hostHandle;     /*!< This instance's related host handle*/
  usb_device_handle deviceHandle; /*!< This instance's related device handle*/
  usb_host_interface_handle
      interfaceHandle; /*!< This instance's related interface handle*/
  usb_host_edgetpu_pipe_t pipes[USB_EDGETPU_ENDPOINT_NUM]; /* pipes */
  usb_host_pipe_handle
      controlPipe; /*!< This instance's related device control pipe*/
  usb_host_transfer_t *controlTransfer; /*!< Ongoing control transfer*/
  transfer_callback_t
      controlCallbackFn;      /*!< control transfer callback function pointer*/
  void *controlCallbackParam; /*!< control transfer callback parameter*/
  usb_host_edgetpu_transfer_status_t controlTransferStatus;
} usb_host_edgetpu_instance_t;

usb_status_t USB_HostEdgeTpuInit(usb_device_handle deviceHandle,
                                 usb_host_class_handle *classHandle);

usb_status_t USB_HostEdgeTpuDeinit(usb_device_handle deviceHandle,
                                   usb_host_class_handle classHandle);

usb_status_t USB_HostEdgeTpuGetStatus(usb_host_class_handle classHandle,
                                      uint8_t *statusData,
                                      transfer_callback_t callbackFn,
                                      void *callbackParam);

usb_status_t USB_HostEdgeTpuDetach(usb_host_class_handle classHandle,
                                   uint16_t timeout,
                                   transfer_callback_t callbackFn,
                                   void *callbackParam);

usb_status_t USB_HostEdgeTpuSetInterface(
    usb_host_class_handle classHandle,
    usb_host_interface_handle interfaceHandle, uint8_t alternateSetting,
    transfer_callback_t callbackFn, void *callbackParam);

usb_status_t USB_HostEdgeTpuBulkOutSend(
    usb_host_edgetpu_instance_t *tpuInstance, uint8_t endPoint, uint8_t *buffer,
    uint16_t length, transfer_callback_t callbackFn, void *callbackParam);

usb_status_t USB_HostEdgeTpuBulkInRecv(usb_host_edgetpu_instance_t *tpuInstance,
                                       uint8_t endPoint, uint8_t *buffer,
                                       uint32_t bufferLength,
                                       transfer_callback_t callbackFn,
                                       void *callbackParam);

usb_status_t USB_HostEdgeTpuControl(usb_host_edgetpu_instance_t *tpuInstance,
                                    usb_setup_struct_t *setupPacket,
                                    uint8_t *buffer,
                                    transfer_callback_t callbackFn,
                                    void *callbackParam);

#ifdef __cplusplus
}
#endif

#endif  // LIBS_TPU_USB_HOST_EDGETPU_H_
