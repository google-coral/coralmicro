#ifndef _LIBS_BASE_NETWORK_H_
#define _LIBS_BASE_NETWORK_H_

#include "third_party/FreeRTOS-Plus-TCP/include/FreeRTOS_IP.h"
#include <cstdlib>

namespace valiant {

class NetworkInterface {
  public:
    virtual bool Initialized() = 0;
    virtual void NetworkEvent(eIPCallbackEvent_t eNetworkEvent) = 0;
    virtual bool TransmitFrame(uint8_t *buffer, uint32_t length) = 0;
    virtual bool ReceiveFrame(uint8_t *buffer, uint32_t length);
};

void InitializeNetworkEEM();

}  // namespace valiant

#endif  // _LIBS_BASE_NETWORK_H_
