#include "libs/base/ethernet.h"

#include "libs/base/gpio.h"
#include "libs/base/utils.h"
#include "third_party/nxp/rt1176-sdk/components/phy/device/phyrtl8211f/fsl_phyrtl8211f.h"
#include "third_party/nxp/rt1176-sdk/components/phy/fsl_phy.h"
#include "third_party/nxp/rt1176-sdk/components/phy/mdio/enet/fsl_enet_mdio.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/port/enet_ethernetif.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/netifapi.h"

namespace coral::micro {

static struct netif netif;
struct netif* eth_netif = nullptr;
static mdio_handle_t mdioHandle = {
    .ops = &enet_ops,
};
static phy_handle_t phyHandle = {
    .phyAddr = 1,
    .mdioHandle = &mdioHandle,
    .ops = &phyrtl8211f_ops,
};

struct netif* GetEthernetInterface() { return eth_netif; }

status_t EthernetPHYWrite(uint32_t phyReg, uint32_t data) {
    return PHY_Write(&phyHandle, phyReg, data);
}

void InitializeEthernet(bool default_iface) {
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyHandle = &phyHandle,
        // MAC Address w/ Google prefix, and blank final 3 octets.
        // They will be populated below.
        .macAddress = {0x00, 0x1A, 0x11, 0x00, 0x00, 0x00},
    };

    // Populate the low bytes of the MAC address with our device's
    // unique ID.
    // In production units, addresses should be in fuses that we can read.
    uint64_t unique_id = coral::micro::utils::GetUniqueID();
    enet_config.macAddress[3] = (unique_id >> 56) & 0xFF;
    enet_config.macAddress[4] = (unique_id >> 48) & 0xFF;
    enet_config.macAddress[5] = (unique_id >> 40) & 0xFF;

    // Set ENET1G TX_CLK to ENET2_CLK_ROOT
    IOMUXC_GPR->GPR5 &= ~IOMUXC_GPR_GPR5_ENET1G_TX_CLK_SEL_MASK;
    // Enable clock output for RGMII TX
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK;
    gpio::SetGpio(gpio::Gpio::kBufferEnable, true);
    // Hold PHY in reset
    gpio::SetGpio(gpio::Gpio::kEthPhyRst, false);
    // Enable 3.3V power for the PHY
    gpio::SetGpio(gpio::Gpio::kBtHostWake, true);
    // Hold in reset for 10ms
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    gpio::SetGpio(gpio::Gpio::kEthPhyRst, true);
    // Delay 72ms for internal circuits to settle
    SDK_DelayAtLeastUs(72000, CLOCK_GetFreq(kCLOCK_CpuClk));

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);

    mdioHandle.resource.csrClock_Hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Bus);
    IP4_ADDR(&netif_ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&netif_netmask, 0, 0, 0, 0);
    IP4_ADDR(&netif_gw, 0, 0, 0, 0);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw,
                       &enet_config, ethernetif1_init, tcpip_input);
    if (default_iface) {
        netifapi_netif_set_default(&netif);
    }
    netifapi_netif_set_up(&netif);
    netifapi_dhcp_start(&netif);
    eth_netif = &netif;
}

}  // namespace coral::micro
