#ifndef _LIBS_BASE_ETHERNET_H_
#define _LIBS_BASE_ETHERNET_H_

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/netifapi.h"

namespace coral::micro {

struct netif* GetEthernetInterface();
void InitializeEthernet(bool default_iface);
status_t EthernetPHYWrite(uint32_t phyReg, uint32_t data);

}  // namespace coral::micro

#endif  // _LIBS_BASE_ETHERNET_H_
