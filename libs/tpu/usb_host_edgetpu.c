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

#include "libs/tpu/usb_host_edgetpu.h"
#include "third_party/modified/nxp/rt1176-sdk/usb_host_config.h"
#include "third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host.h"

static usb_status_t USB_HostEdgeTpuOpenInterface(usb_host_edgetpu_instance_t *tpuInstance);
static usb_status_t USB_HostEdgeTpuOpenDataInterface(usb_host_edgetpu_instance_t *tpuInstance);


void USB_HostEdgeTpuSetInterfaceCallback(void *param, usb_host_transfer_t *transfer, usb_status_t status)
{
    usb_host_edgetpu_instance_t *tpuInstance = (usb_host_edgetpu_instance_t*)param;
    tpuInstance->controlTransfer = NULL;
    if (status == kStatus_USB_Success) {
        status = USB_HostEdgeTpuOpenInterface(tpuInstance);
    }

    if (tpuInstance->controlCallbackFn != NULL)
    {
        tpuInstance->controlCallbackFn(tpuInstance->controlCallbackParam, NULL, 0, status);
    }
    USB_HostFreeTransfer(tpuInstance->hostHandle, transfer);
}

usb_status_t USB_HostEdgeTpuInit(usb_device_handle deviceHandle, usb_host_class_handle *classHandle)
{
    uint32_t infoValue;
    /* malloc edgetpu class instance */
    usb_host_edgetpu_instance_t *tpuInstance =
        (usb_host_edgetpu_instance_t *)OSA_MemoryAllocate(sizeof(usb_host_edgetpu_instance_t));
    if (tpuInstance == NULL)
    {
        return kStatus_USB_AllocFail;
    }

    /* initialize tpu instance */
    tpuInstance->deviceHandle = deviceHandle;
    tpuInstance->interfaceHandle = NULL;
    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetHostHandle, &infoValue);
    tpuInstance->hostHandle = (usb_host_handle)infoValue;
    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceControlPipe, &infoValue);
    tpuInstance->controlPipe = (usb_host_pipe_handle)infoValue;
    *classHandle = tpuInstance;
    return kStatus_USB_Success;
}

usb_status_t USB_HostEdgeTpuDeinit(usb_device_handle deviceHandle, usb_host_class_handle classHandle)
{
    usb_status_t status = kStatus_USB_Success;
    usb_host_edgetpu_instance_t *tpuInstance = (usb_host_edgetpu_instance_t *)classHandle;

    if (deviceHandle == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    if (classHandle != NULL)
    {
        for (int i = 0; i < USB_EDGETPU_ENDPOINT_NUM; i++)
        {
            if (tpuInstance->pipes[i].pipeHandle != NULL)
            {
                status = USB_HostCancelTransfer(tpuInstance->hostHandle, tpuInstance->pipes[i].pipeHandle, NULL);
                status = USB_HostClosePipe(tpuInstance->hostHandle, tpuInstance->pipes[i].pipeHandle);
                if (status != kStatus_USB_Success)
                {
                    usb_echo("Error when closing pipe\r\n");
                }
                tpuInstance->pipes[i].pipeHandle = NULL;
            }
        }

        if ((tpuInstance->controlPipe != NULL) && (tpuInstance->controlTransfer != NULL))
        {
            status = USB_HostCancelTransfer(tpuInstance->hostHandle, tpuInstance->controlPipe, tpuInstance->controlTransfer);
        }
        USB_HostCloseDeviceInterface(deviceHandle, tpuInstance->interfaceHandle);
        OSA_MemoryFree(tpuInstance);
    }
    else
    {
        USB_HostCloseDeviceInterface(deviceHandle, NULL);
    }
    return status;
}

usb_status_t USB_HostEdgeTpuGetStatus(usb_host_class_handle classHandle,
                                  uint8_t *statusData,
                                  transfer_callback_t callbackFn,
                                  void *callbackParam)
{
    usb_host_edgetpu_instance_t *tpuInstance = (usb_host_edgetpu_instance_t *)classHandle;
    if (tpuInstance == NULL) {
        return kStatus_USB_Error;
    }
    for (int i = 0; i < USB_EDGETPU_ENDPOINT_NUM; i++)
    {
        if (tpuInstance->pipes[i].connected == false && i != USB_EDGETPU_INTERRRUPT_ENDPOINT_INDEX) {
            return kStatus_USB_Error;
        }
    }
    return kStatus_USB_Success;
}

usb_status_t USB_HostEdgeTpuDetach(usb_host_class_handle classHandle,
                               uint16_t timeout,
                               transfer_callback_t callbackFn,
                               void *callbackParam)
{
    return kStatus_USB_Success;
}

static usb_status_t USB_HostEdgeTpuOpenInterface(usb_host_edgetpu_instance_t *tpuInstance)
{
    return USB_HostEdgeTpuOpenDataInterface(tpuInstance);
}

usb_status_t USB_HostEdgeTpuSetInterface(usb_host_class_handle classHandle,
                                     usb_host_interface_handle interfaceHandle,
                                     uint8_t alternateSetting,
                                     transfer_callback_t callbackFn,
                                     void *callbackParam)
{
    usb_status_t status;
    usb_host_edgetpu_instance_t *tpuInstance = (usb_host_edgetpu_instance_t *)classHandle;
    usb_host_transfer_t *transfer;
    uint8_t ep_index = 0;

    if (classHandle == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    tpuInstance->interfaceHandle = interfaceHandle;

    status = USB_HostOpenDeviceInterface(tpuInstance->deviceHandle, interfaceHandle);
    if (status != kStatus_USB_Success)
    {
        return status;
    }
    for (ep_index = 0; ep_index < USB_EDGETPU_ENDPOINT_NUM; ++ep_index) {
        USB_HostCancelTransfer(tpuInstance->hostHandle, tpuInstance->pipes[ep_index].pipeHandle, NULL);
    }


    if (alternateSetting == 0 )
    {
        if (callbackFn != NULL)
        {
            status = USB_HostEdgeTpuOpenInterface(tpuInstance);
            callbackFn(callbackParam, NULL, 0, status);
        }
    }
    else
    {
        if (USB_HostMallocTransfer(tpuInstance->hostHandle, &transfer) != kStatus_USB_Success)
        {
            return kStatus_USB_Error;
        }
        tpuInstance->controlCallbackFn = callbackFn;
        tpuInstance->controlCallbackParam = callbackParam;
        transfer->callbackFn = USB_HostEdgeTpuSetInterfaceCallback;
        transfer->callbackParam = tpuInstance;
        transfer->setupPacket->bRequest = USB_REQUEST_STANDARD_SET_INTERFACE;
        transfer->setupPacket->bmRequestType = USB_REQUEST_TYPE_RECIPIENT_INTERFACE;
        transfer->setupPacket->wIndex = USB_SHORT_TO_LITTLE_ENDIAN(
            ((usb_host_interface_t *)tpuInstance->interfaceHandle)->interfaceDesc->bInterfaceNumber);
        transfer->setupPacket->wValue  = USB_SHORT_TO_LITTLE_ENDIAN(alternateSetting);
        transfer->setupPacket->wLength = 0;
        transfer->transferBuffer = NULL;
        transfer->transferLength = 0;
        status = USB_HostSendSetup(tpuInstance->hostHandle, tpuInstance->controlPipe, transfer);

        if (status == kStatus_USB_Success)
        {
            tpuInstance->controlTransfer = transfer;
        }
        else
        {
            USB_HostFreeTransfer(tpuInstance->hostHandle, transfer);
        }
    }
    return status;
}



static usb_status_t USB_HostEdgeTpuOpenDataInterface(usb_host_edgetpu_instance_t *tpuInstance)
{
    usb_status_t status;
    uint8_t ep_index = 0;
    usb_host_pipe_init_t pipeInit;
    usb_descriptor_endpoint_t *ep_desc = NULL;
    usb_host_interface_t *interfaceHandle;

    for (ep_index = 0; ep_index < USB_EDGETPU_ENDPOINT_NUM; ++ep_index) {
        if (tpuInstance->pipes[ep_index].pipeHandle != NULL) {
            status = USB_HostClosePipe(tpuInstance->hostHandle, tpuInstance->pipes[ep_index].pipeHandle);
            tpuInstance->pipes[ep_index].connected = false;
        }
    }

    status = USB_HostOpenDeviceInterface(tpuInstance->deviceHandle, tpuInstance->interfaceHandle);
    if (status != kStatus_USB_Success)
    {
        return status;
    }
    /* open interface pipes */
    interfaceHandle = (usb_host_interface_t *)tpuInstance->interfaceHandle;

    if ((interfaceHandle->epCount) != USB_EDGETPU_ENDPOINT_NUM) {
        /* Numbder of bulk endpoints is expected */
        return kStatus_USB_Error;
    }

    for (ep_index = 0; ep_index < interfaceHandle->epCount; ++ep_index)
    {
        ep_desc = interfaceHandle->epList[ep_index].epDesc;
        if (((ep_desc->bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
             USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN) &&
            ((ep_desc->bmAttributes & USB_DESCRIPTOR_ENDPOINT_ATTRIBUTE_TYPE_MASK) == USB_ENDPOINT_BULK))
        {
            pipeInit.devInstance = tpuInstance->deviceHandle;
            pipeInit.pipeType = USB_ENDPOINT_BULK;
            pipeInit.direction = USB_IN;
            pipeInit.endpointAddress = (ep_desc->bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_NUMBER_MASK);
            pipeInit.interval = ep_desc->bInterval;
            pipeInit.maxPacketSize = USB_EDGETPU_BULK_IN_PACKET_SIZE;
            pipeInit.numberPerUframe = (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(ep_desc->wMaxPacketSize) &
                                        USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_MULT_TRANSACTIONS_MASK);
            pipeInit.nakCount = USB_HOST_CONFIG_MAX_NAK;

            tpuInstance->pipes[ep_index].packetSize = pipeInit.maxPacketSize;
            tpuInstance->pipes[ep_index].pipeType = USB_ENDPOINT_BULK;
            tpuInstance->pipes[ep_index].endPoint = pipeInit.endpointAddress;
            tpuInstance->pipes[ep_index].direction = USB_IN;
            tpuInstance->pipes[ep_index].connected = true;

            status = USB_HostOpenPipe(tpuInstance->hostHandle, &tpuInstance->pipes[ep_index].pipeHandle, &pipeInit);
            if (status != kStatus_USB_Success)
            {
                tpuInstance->pipes[ep_index].connected = false;
                return kStatus_USB_Error;
            }
        }
        else if (((ep_desc->bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                  USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_OUT) &&
                 ((ep_desc->bmAttributes & USB_DESCRIPTOR_ENDPOINT_ATTRIBUTE_TYPE_MASK) == USB_ENDPOINT_BULK))
        {
            pipeInit.devInstance = tpuInstance->deviceHandle;
            pipeInit.pipeType = USB_ENDPOINT_BULK;
            pipeInit.direction = USB_OUT;
            pipeInit.endpointAddress = (ep_desc->bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_NUMBER_MASK);
            pipeInit.interval = ep_desc->bInterval;
            pipeInit.maxPacketSize = USB_EDGETPU_BULK_OUT_PACKET_SIZE; // Limited by the EdgeTpu HW.
            pipeInit.numberPerUframe = (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(ep_desc->wMaxPacketSize) &
                                        USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_MULT_TRANSACTIONS_MASK);
            pipeInit.nakCount = USB_HOST_CONFIG_MAX_NAK;

            tpuInstance->pipes[ep_index].packetSize = pipeInit.maxPacketSize;
            tpuInstance->pipes[ep_index].pipeType = USB_ENDPOINT_BULK;
            tpuInstance->pipes[ep_index].endPoint = pipeInit.endpointAddress;
            tpuInstance->pipes[ep_index].direction = USB_OUT;
            tpuInstance->pipes[ep_index].connected = true;

            status = USB_HostOpenPipe(tpuInstance->hostHandle, &tpuInstance->pipes[ep_index].pipeHandle, &pipeInit);
            if (status != kStatus_USB_Success)
            {
                tpuInstance->pipes[ep_index].connected = false;
                return kStatus_USB_Error;
            }
        }
         else if (((ep_desc->bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                  USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN) &&
                 ((ep_desc->bmAttributes & USB_DESCRIPTOR_ENDPOINT_ATTRIBUTE_TYPE_MASK) == USB_ENDPOINT_INTERRUPT))
        {
            pipeInit.devInstance = tpuInstance->deviceHandle;
            pipeInit.pipeType = USB_ENDPOINT_INTERRUPT;
            pipeInit.direction = USB_IN;
            pipeInit.endpointAddress = (ep_desc->bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_NUMBER_MASK);
            pipeInit.interval = ep_desc->bInterval;
            pipeInit.maxPacketSize = (uint16_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(ep_desc->wMaxPacketSize) &
                                                USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_SIZE_MASK);
            pipeInit.numberPerUframe = (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(ep_desc->wMaxPacketSize) &
                                        USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_MULT_TRANSACTIONS_MASK);
            pipeInit.nakCount = USB_HOST_CONFIG_MAX_NAK;

            tpuInstance->pipes[ep_index].packetSize = pipeInit.maxPacketSize;
            tpuInstance->pipes[ep_index].pipeType = USB_ENDPOINT_INTERRUPT;
            tpuInstance->pipes[ep_index].endPoint = pipeInit.endpointAddress;
            tpuInstance->pipes[ep_index].direction = USB_IN;
            tpuInstance->pipes[ep_index].connected = true;

            status = USB_HostOpenPipe(tpuInstance->hostHandle, &tpuInstance->pipes[ep_index].pipeHandle, &pipeInit);
            if (status != kStatus_USB_Success)
            {
                tpuInstance->pipes[ep_index].connected = false;
                // TODO(henryherman) Failing to open the interrupt endpoint due to bandwidth check.
                //return kStatus_USB_Error;
                continue;
            }
        }
        else
        {
        }
    }
    return kStatus_USB_Success;
}


static int8_t USB_HostEdgeTpuGetPipeIndexFromEndpoint(usb_host_edgetpu_instance_t *tpuInstance,
                                                      uint8_t endPoint,
                                                      uint8_t direction)
{
    for (int i = 0; i < USB_EDGETPU_ENDPOINT_NUM; i++)
    {
        if (tpuInstance->pipes[i].endPoint == endPoint &&
            tpuInstance->pipes[i].direction == direction) {
            return i;
        }
    }
    return -1;
}


static void USB_HostEdgeTpuPipeCallback(void *param,
                                           usb_host_transfer_t *transfer,
                                           usb_status_t status)
{
    usb_host_edgetpu_instance_t *tpuInstance = (usb_host_edgetpu_instance_t *)param;
    int8_t index = -1;
    for (int i = 0; i < USB_EDGETPU_ENDPOINT_NUM; i++)
    {
        if (tpuInstance->pipes[i].activeTransfer == transfer) {
            index = i;
            break;
        }
    }
    if (!(index < 0)) {
        usb_host_edgetpu_pipe_t *edgeTpuPipe = &tpuInstance->pipes[index];
        if (edgeTpuPipe->callbackFn != NULL) {
            edgeTpuPipe->callbackFn(edgeTpuPipe->callbackParam, transfer->transferBuffer, transfer->transferSofar, status);
        }
        edgeTpuPipe->activeTransfer = NULL;
        edgeTpuPipe->transferStatus = USB_EDGETPU_TRANSFER_READY;
    }
    USB_HostFreeTransfer(tpuInstance->hostHandle, transfer);
}


usb_status_t USB_HostEdgeTpuBulkOutSend(usb_host_edgetpu_instance_t *tpuInstance,
                                            uint8_t endPoint,
                                            uint8_t* buffer,
                                            uint16_t length,
                                            transfer_callback_t callbackFn,
                                            void *callbackParam)
{
    usb_host_transfer_t *transfer;

    // Determine index of pipe to endpoint
    int8_t index = USB_HostEdgeTpuGetPipeIndexFromEndpoint(tpuInstance, endPoint, USB_OUT);
    if (index < 0)
    {
        return kStatus_USB_InvalidParameter;
    }
    usb_host_edgetpu_pipe_t *pipe = &tpuInstance->pipes[index];

    // Check endpoint is BULK
    if (pipe->pipeType != USB_ENDPOINT_BULK)
    {
        return kStatus_USB_InvalidParameter;
    }

    if (USB_HostMallocTransfer(tpuInstance->hostHandle, &transfer) != kStatus_USB_Success)
    {
        return kStatus_USB_Error;
    }

    transfer->transferBuffer = buffer;
    transfer->transferLength = length;
    transfer->callbackFn = USB_HostEdgeTpuPipeCallback;
    transfer->callbackParam = tpuInstance;
    transfer->direction = USB_OUT;
    pipe->transferStatus = USB_EDGETPU_TRANSFER_BUSY;
    pipe->callbackFn = callbackFn;
    pipe->callbackParam = callbackParam;
    pipe->activeTransfer = transfer;

    if (USB_HostSend(tpuInstance->hostHandle, pipe->pipeHandle, transfer) != kStatus_USB_Success)
    {
        pipe->transferStatus = USB_EDGETPU_TRANSFER_READY;
        pipe->activeTransfer = NULL;
        USB_HostFreeTransfer(tpuInstance->hostHandle, transfer);
        return kStatus_USB_Error;
    }
    return kStatus_USB_Success;
}


usb_status_t USB_HostEdgeTpuBulkInRecv(usb_host_edgetpu_instance_t *tpuInstance,
                                 uint8_t endPoint,
                                 uint8_t *buffer,
                                 uint32_t length,
                                 transfer_callback_t callbackFn,
                                 void *callbackParam)
{
    usb_host_transfer_t *transfer;

    // Determine index of pipe from endpoint
    int8_t index = USB_HostEdgeTpuGetPipeIndexFromEndpoint(tpuInstance, endPoint, USB_IN);
    if (index < 0)
    {
        return kStatus_USB_InvalidParameter;
    }
    usb_host_edgetpu_pipe_t *pipe = &tpuInstance->pipes[index];

    // Check endpoint is BULK
    if (pipe->pipeType != USB_ENDPOINT_BULK)
    {
        return kStatus_USB_InvalidParameter;
    }

    // Allocate memory for transfer
    if (USB_HostMallocTransfer(tpuInstance->hostHandle, &transfer) != kStatus_USB_Success)
    {
        return kStatus_USB_Error;
    }

    pipe->callbackFn = callbackFn;
    pipe->callbackParam = callbackParam;
    pipe->activeTransfer = transfer;
    pipe->transferStatus = USB_EDGETPU_TRANSFER_BUSY;
    transfer->transferBuffer = buffer;
    transfer->transferLength = length;
    transfer->callbackFn = USB_HostEdgeTpuPipeCallback;
    transfer->callbackParam = tpuInstance;
    transfer->direction = USB_IN;

    if (USB_HostRecv(tpuInstance->hostHandle, pipe->pipeHandle, transfer) != kStatus_USB_Success)
    {
        pipe->transferStatus = USB_EDGETPU_TRANSFER_READY;
        pipe->activeTransfer = NULL;
        USB_HostFreeTransfer(tpuInstance->hostHandle, transfer);
        return kStatus_USB_Error;
    }
    return kStatus_USB_Success;
}


static void USB_HostEdgeTpuControlPipeCallback(void *param, usb_host_transfer_t *transfer, usb_status_t status)
{
    usb_host_edgetpu_instance_t *tpuInstance = (usb_host_edgetpu_instance_t *)param;

    if (tpuInstance->controlCallbackFn != NULL)
    {
        tpuInstance->controlCallbackFn(tpuInstance->controlCallbackParam, transfer->transferBuffer,
                                       transfer->transferSofar, status);
    }
    USB_HostFreeTransfer(tpuInstance->hostHandle, transfer);
    tpuInstance->controlTransferStatus = USB_EDGETPU_TRANSFER_BUSY;
}


usb_status_t USB_HostEdgeTpuControl(usb_host_edgetpu_instance_t *tpuInstance,
                                               usb_setup_struct_t *setupPacket,
                                               uint8_t* buffer,
                                               transfer_callback_t callbackFn,
                                               void *callbackParam)
{
    usb_host_transfer_t *transfer;
    if (USB_HostMallocTransfer(tpuInstance->hostHandle, &transfer) != kStatus_USB_Success)
    {
        return kStatus_USB_Error;
    }
    tpuInstance->controlCallbackFn = callbackFn;
    tpuInstance->controlCallbackParam = callbackParam;
    transfer->transferBuffer = buffer;
    transfer->transferLength = setupPacket->wLength;
    transfer->callbackFn = USB_HostEdgeTpuControlPipeCallback;
    transfer->callbackParam = tpuInstance;
    transfer->setupPacket->bRequest = setupPacket->bRequest;
    transfer->setupPacket->bmRequestType = setupPacket->bmRequestType;
    transfer->setupPacket->wValue = USB_SHORT_TO_LITTLE_ENDIAN(setupPacket->wValue);
    transfer->setupPacket->wIndex = USB_SHORT_TO_LITTLE_ENDIAN(setupPacket->wIndex);
    transfer->setupPacket->wLength = USB_SHORT_TO_LITTLE_ENDIAN(setupPacket->wLength);

    tpuInstance->controlTransferStatus = USB_EDGETPU_TRANSFER_BUSY;
    if (USB_HostSendSetup(tpuInstance->hostHandle,
                          tpuInstance->controlPipe, transfer) != kStatus_USB_Success)
    {
        USB_HostFreeTransfer(tpuInstance->hostHandle, transfer);
        tpuInstance->controlTransferStatus = USB_EDGETPU_TRANSFER_BUSY;
        return kStatus_USB_Error;
    }
    tpuInstance->controlTransfer = transfer;
    return kStatus_USB_Success;
}


