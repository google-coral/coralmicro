#include "libs/base/network.h"
#include "libs/CdcEem/cdc_eem.h"
#include "libs/base/random.h"
#include "libs/tasks/UsbDeviceTask/usb_device_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/FreeRTOS-Plus-TCP/include/FreeRTOS_IP.h"
#include "third_party/FreeRTOS-Plus-TCP/include/NetworkBufferManagement.h"
#include "third_party/FreeRTOS-Plus-TCP/include/FreeRTOS_IP_Private.h"

#include <cstdio>
#include <cstring>
#include <functional>

using namespace std::placeholders;

namespace valiant {

static NetworkInterface *network_interface = nullptr;
static CdcEem cdc_eem_;

void InitializeNetworkEEM() {
    cdc_eem_.Init(
            valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_descriptor_value(),
            valiant::UsbDeviceTask::GetSingleton()->next_interface_value());
    valiant::UsbDeviceTask::GetSingleton()->AddDevice(cdc_eem_.config_data(),
            std::bind(&valiant::CdcEem::SetClassHandle, &cdc_eem_, _1),
            std::bind(&valiant::CdcEem::HandleEvent, &cdc_eem_, _1, _2));
    network_interface = &cdc_eem_;
}

bool NetworkInterface::ReceiveFrame(uint8_t *buffer, uint32_t length) {
    NetworkBufferDescriptor_t *pxBufferDescriptor;
    pxBufferDescriptor = pxGetNetworkBufferWithDescriptor(length, 0);
    if (pxBufferDescriptor) {
        memcpy(pxBufferDescriptor->pucEthernetBuffer, buffer, length);
        pxBufferDescriptor->xDataLength = length;

        if (eConsiderFrameForProcessing(pxBufferDescriptor->pucEthernetBuffer) == eProcessBuffer) {
            IPStackEvent_t xRxEvent;
            xRxEvent.eEventType = eNetworkRxEvent;
            xRxEvent.pvData = (void*)pxBufferDescriptor;
            if (xSendEventStructToIPTask(&xRxEvent, 0) == pdFALSE) {
                vReleaseNetworkBufferAndDescriptor(pxBufferDescriptor);
                iptraceETHERNET_RX_EVENT_LOST();
                return false;
            } else {
                iptraceNETWORK_INTERFACE_RECEIVE();
            }
        } else {
            vReleaseNetworkBufferAndDescriptor(pxBufferDescriptor);
            return false;
        }
    } else {
        iptraceETHERNET_RX_EVENT_LOST();
        return false;
    }
    return true;
}

}  // namespace valiant

extern "C" BaseType_t xNetworkInterfaceInitialise(void) {
    if (valiant::network_interface)
        return (valiant::network_interface->Initialized() ? pdPASS : pdFALSE);
    return pdFALSE;
}

extern "C" void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent) {
    if (valiant::network_interface)
        valiant::network_interface->NetworkEvent(eNetworkEvent);
}

extern "C" BaseType_t xNetworkInterfaceOutput(
        NetworkBufferDescriptor_t * const pxDescriptor,
        BaseType_t xReleaseAfterSend) {

    if (!valiant::network_interface)
        return pdFALSE;

    bool status = valiant::network_interface->TransmitFrame(pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength);
    if (!status) {
      printf("xNetworkInterfaceOutput status %d\r\n", status);
    }

    iptraceNETWORK_INTERFACE_TRANSMIT();
    if (xReleaseAfterSend != pdFALSE) {
        vReleaseNetworkBufferAndDescriptor(pxDescriptor);
    }
    return pdTRUE;
}

extern "C" BaseType_t xApplicationGetRandomNumber(uint32_t * pulNumber) {
    return valiant::Random::GetSingleton()->GetRandomNumber(pulNumber, sizeof(*pulNumber)) ? pdTRUE : pdFALSE;
}

// TODO(atv): Implement something similar to suggestions in RFC6528
extern "C" uint32_t ulApplicationGetNextSequenceNumber(
        uint32_t ulSourceAddress, uint16_t usSourcePort,
        uint32_t ulDestinationAddress, uint16_t usDestinationPort) {
    uint32_t random;
    xApplicationGetRandomNumber(&random);
    return random;
}
