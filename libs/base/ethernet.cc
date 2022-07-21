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

#include "libs/base/ethernet.h"

#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/utils.h"
#include "third_party/nxp/rt1176-sdk/components/phy/device/phyrtl8211f/fsl_phyrtl8211f.h"
#include "third_party/nxp/rt1176-sdk/components/phy/mdio/enet/fsl_enet_mdio.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/port/enet_ethernetif.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/netifapi.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/prot/dhcp.h"

namespace coralmicro {
namespace {
struct netif g_netif;
struct netif* g_eth_netif = nullptr;

mdio_handle_t g_mdio_handle = {
    .ops = &enet_ops,
};

phy_handle_t g_phy_handle = {
    .phyAddr = 1,
    .mdioHandle = &g_mdio_handle,
    .ops = &phyrtl8211f_ops,
};

constexpr uint32_t kBasicModeControlReg = 0;
constexpr uint32_t kGigBaseControlReg = 9;
constexpr uint32_t kRXCSSCReg = 19;
constexpr uint32_t kSystemClockSSCInitReg = 23;
constexpr uint32_t kSystemClockSSCReg = 25;
constexpr uint32_t kPageSelectReg = 31;

status_t EthernetPHYEnableSSC(bool auto_neg) {
  uint32_t reg;
  status_t status;
  const uint32_t reset = auto_neg ? 0x9200 : 0x8000;
  // Enable RXC SSC
  status = EthernetPhyWrite(kPageSelectReg, 0x0C44);
  status |= EthernetPhyWrite(kRXCSSCReg, 0x5F00);
  status |= EthernetPhyWrite(kPageSelectReg, 0x0);
  status |= EthernetPhyWrite(kBasicModeControlReg, reset);

  // Enable System Clock SSC
  status |= EthernetPhyWrite(kPageSelectReg, 0x0C44);
  status |= EthernetPhyWrite(kSystemClockSSCInitReg, 0x4F00);
  status |= EthernetPhyWrite(kPageSelectReg, 0x0A43);
  status |= PHY_Read(&g_phy_handle, kSystemClockSSCReg, &reg);
  status |= EthernetPhyWrite(kSystemClockSSCReg, reg | (1U << 3));
  status |= EthernetPhyWrite(kPageSelectReg, 0x0);
  status |= EthernetPhyWrite(kBasicModeControlReg, reset);

  // Disable CLK_OUT
  status |= EthernetPhyWrite(kPageSelectReg, 0x0A43);
  status |= PHY_Read(&g_phy_handle, kSystemClockSSCReg, &reg);
  status |= EthernetPhyWrite(kSystemClockSSCReg, reg & ~1U);
  status |= EthernetPhyWrite(kPageSelectReg, 0x0);
  status |= EthernetPhyWrite(kBasicModeControlReg, reset);
  return status;
}
}  // namespace

struct netif* EthernetGetInterface() { return g_eth_netif; }

status_t EthernetPhyWrite(uint32_t phy_reg, uint32_t data) {
  return PHY_Write(&g_phy_handle, phy_reg, data);
}

void EthernetInit(bool default_iface) {
  ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;

  phy_config_t phy_config = {
      .phyAddr = 1,
      .duplex = kPHY_FullDuplex,
      .autoNeg = true,
      .enableEEE = true,
  };

  int speed = EthernetGetSpeed();
  switch (speed) {
    case 1000:
      // Gigabit Ethernet is unsupported due to EMI concerns. If this is
      // not a concern, this assert should be removed and 1000 should be
      // added as a choice for ethernet_speed in scripts/flashtool.py.
      assert(!"1G Ethernet is unsupported");
      phy_config.speed = kPHY_Speed1000M;
      break;
    case 100:
      phy_config.speed = kPHY_Speed100M;
      break;
    case 10:
      phy_config.speed = kPHY_Speed10M;
      break;
    default:
      printf("Invalid ethernet speed, assuming 100M\r\n");
      phy_config.speed = kPHY_Speed100M;
  }

  ethernetif_config_t enet_config = {
      .phyHandle = &g_phy_handle,
      .phyConfig = &phy_config,
      // MAC Address w/ Google prefix, and blank final 3 octets.
      // They will be populated below.
      .macAddress = {0x00, 0x1A, 0x11, 0x00, 0x00, 0x00},
  };

  // Populate the low bytes of the MAC address with our device's
  // unique ID.
  // In production units, addresses should be in fuses that we can read.
  uint64_t unique_id = coralmicro::utils::GetUniqueId();
  enet_config.macAddress[3] = (unique_id >> 56) & 0xFF;
  enet_config.macAddress[4] = (unique_id >> 48) & 0xFF;
  enet_config.macAddress[5] = (unique_id >> 40) & 0xFF;

  // Set ENET1G TX_CLK to ENET2_CLK_ROOT
  IOMUXC_GPR->GPR5 &= ~IOMUXC_GPR_GPR5_ENET1G_TX_CLK_SEL_MASK;
  // Enable clock output for RGMII TX
  IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK;
  // Hold PHY in reset
  GpioSet(Gpio::kEthPhyRst, false);
  // Enable 3.3V power for the PHY
  GpioSet(Gpio::kBtHostWake, true);
  // Hold in reset for 10ms
  SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
  GpioSet(Gpio::kEthPhyRst, true);
  // Delay 72ms for internal circuits to settle
  SDK_DelayAtLeastUs(72000, CLOCK_GetFreq(kCLOCK_CpuClk));

  EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
  EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);

  g_mdio_handle.resource.csrClock_Hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Bus);
  IP4_ADDR(&netif_ipaddr, 0, 0, 0, 0);
  IP4_ADDR(&netif_netmask, 0, 0, 0, 0);
  IP4_ADDR(&netif_gw, 0, 0, 0, 0);

  if (EthernetPHYEnableSSC(phy_config.autoNeg)) {
    printf("Failed enabling PHY SSC, proceeding.\r\n");
  }

  netifapi_netif_add(&g_netif, &netif_ipaddr, &netif_netmask, &netif_gw,
                     &enet_config, ethernetif1_init, tcpip_input);
  if (default_iface) {
    netifapi_netif_set_default(&g_netif);
  }

  netifapi_netif_set_up(&g_netif);
  netifapi_dhcp_start(&g_netif);
  g_eth_netif = &g_netif;
}

std::optional<std::string> EthernetGetIp() {
  if (!g_eth_netif) {
    return std::nullopt;
  }
  while (true) {
    auto* dhcp = netif_dhcp_data(g_eth_netif);
    if (dhcp->state == DHCP_STATE_BOUND) {
      break;
    }
    taskYIELD();
  }

  return ip4addr_ntoa(netif_ip4_addr(g_eth_netif));
}

int EthernetGetSpeed() {
  std::string ethernet_speed;
  uint16_t speed;
  if (!LfsReadFile("/ethernet_speed", &ethernet_speed)) {
    printf("Failed to read ethernet speed, assuming 100M.\r\n");
    speed = 100;
  } else {
    speed = *reinterpret_cast<uint16_t*>(ethernet_speed.data());
  }
  return speed;
}

}  // namespace coralmicro
