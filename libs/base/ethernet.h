#ifndef _LIBS_BASE_ETHERNET_H_
#define _LIBS_BASE_ETHERNET_H_

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/netifapi.h"

namespace valiant {

struct netif* GetEthernetInterface();
void InitializeEthernet(bool default_iface);

}  // namespace valiant

#endif  // _LIBS_BASE_ETHERNET_H_
