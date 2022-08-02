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

#include "libs/nxp/rt1176-sdk/board_hardware.h"

#include <stdio.h>
#include <string.h>

#include "third_party/modified/nxp/rt1176-sdk/board.h"
#include "third_party/modified/nxp/rt1176-sdk/pin_mux.h"
#include "third_party/nxp/rt1176-sdk/components/flash/nand/flexspi/fsl_flexspi_nand_flash.h"
#include "third_party/nxp/rt1176-sdk/components/flash/nand/fsl_nand_flash.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_enet.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_semc.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"

#if __CORTEX_M == 7
extern uint32_t __SDRAM_ROM;
extern uint32_t __sdram_data_start__;
extern uint32_t __sdram_data_end__;
extern uint32_t __sdram_bss_start__;
extern uint32_t __sdram_bss_end__;
static caam_job_ring_interface_t caam_job_ring_0;
static caam_job_ring_interface_t caam_job_ring_1;
static caam_job_ring_interface_t caam_job_ring_2;
static caam_job_ring_interface_t caam_job_ring_3;
#endif

static nand_handle_t nand_handle;
nand_handle_t *BOARD_GetNANDHandle(void) { return &nand_handle; }

void BOARD_ENETFlexibleConfigure(enet_config_t *config) {
  config->miiMode = kENET_RgmiiMode;
}

extern mcmgr_status_t MCMGR_EarlyInit(void) __attribute__((weak));
extern mcmgr_status_t MCMGR_Init(void) __attribute__((weak));

void SystemInitHook(void) { MCMGR_EarlyInit(); }

#if __CORTEX_M == 7
/* Copied in from SDK sample for EVK. */
bool BOARD_InitSEMC(void) {
  semc_config_t config;
  semc_sdram_config_t sdramconfig;
  uint32_t clockFrq = CLOCK_GetRootClockFreq(kCLOCK_Root_Semc);

  /* Initializes the MAC configure structure to zero. */
  memset(&config, 0, sizeof(semc_config_t));
  memset(&sdramconfig, 0, sizeof(semc_sdram_config_t));

  /* Initialize SEMC. */
  SEMC_GetDefaultConfig(&config);
  config.dqsMode = kSEMC_Loopbackdqspad; /* For more accurate timing. */
  SEMC_Init(SEMC, &config);

  /* Configure SDRAM. */
  sdramconfig.csxPinMux = kSEMC_MUXCSX0;
  sdramconfig.address = 0x80000000;
  sdramconfig.memsize_kbytes = 64 * 1024; /* 64MB = 64*1024*1KBytes*/
  sdramconfig.portSize = kSEMC_PortSize16Bit;
  sdramconfig.burstLen = kSEMC_Sdram_BurstLen8;
  sdramconfig.columnAddrBitNum = kSEMC_SdramColunm_10bit;
  sdramconfig.casLatency = kSEMC_LatencyThree;
  sdramconfig.tPrecharge2Act_Ns = 18; /* tRP */
  sdramconfig.tAct2ReadWrite_Ns = 18; /* tRCD */
  sdramconfig.tRefreshRecovery_Ns =
      120; /* Use the maximum of the (Trfc , Txsr). */
  sdramconfig.tWriteRecovery_Ns = 15; /* tWR */
  sdramconfig.tCkeOff_Ns =
      42; /* The minimum cycle of SDRAM CLK off state. CKE is off in self
             refresh at a minimum period tRAS.*/
  sdramconfig.tAct2Prechage_Ns = 42; /* tRAS */
  sdramconfig.tSelfRefRecovery_Ns = 70;
  sdramconfig.tRefresh2Refresh_Ns = 60;
  sdramconfig.tAct2Act_Ns = 30; /* tRC/tRDD */
  sdramconfig.tPrescalePeriod_Ns = 160 * (1000000000 / clockFrq);
  sdramconfig.refreshPeriod_nsPerRow = 64 * 1000000 / 8192; /* 64ms/8192 */
  sdramconfig.refreshUrgThreshold = sdramconfig.refreshPeriod_nsPerRow;
  sdramconfig.refreshBurstLen = 1;
  sdramconfig.delayChain = 2;

  return SEMC_ConfigureSDRAM(SEMC, kSEMC_SDRAM_CS0, &sdramconfig, clockFrq) ==
         kStatus_Success;
}

void BOARD_InitCAAM(void) {
  caam_config_t config;
  CAAM_GetDefaultConfig(&config);
  config.jobRingInterface[0] = &caam_job_ring_0;
  config.jobRingInterface[1] = &caam_job_ring_1;
  config.jobRingInterface[2] = &caam_job_ring_2;
  config.jobRingInterface[3] = &caam_job_ring_3;
  CAAM_Init(CAAM, &config);
}
#endif  // __CORTEX_M == 7

void BOARD_InitNAND(void) {
  flexspi_mem_config_t mem_config = {
      .deviceConfig =
          {
              .flexspiRootClk = CLOCK_GetRootClockFreq(kCLOCK_Root_Flexspi1),
              .flashSize = 0x20000,
              .CSIntervalUnit = kFLEXSPI_CsIntervalUnit1SckCycle,
              .CSInterval = 2,
              .CSHoldTime = 3,
              .CSSetupTime = 3,
              .dataValidTime = 0,
              .columnspace = 12,
              .enableWordAddress = 0,
              .AWRSeqIndex = 0,
              .AWRSeqNumber = 0,
              .ARDSeqIndex = 0,
              .ARDSeqNumber = 0,
              .AHBWriteWaitUnit = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
              .AHBWriteWaitInterval = 0,
          },
      .devicePort = kFLEXSPI_PortA1,
      .dataBytesPerPage = 2048,
      .bytesInPageSpareArea = 2048,
      .pagesPerBlock = 64,
      .busyOffset = 0,
      .busyBitPolarity = 0,
      .eccStatusMask = 0x30,
      .eccFailureMask = 0x20,
  };
  nand_config_t nand_config = {
      .memControlConfig = &mem_config,
      .driverBaseAddr = FLEXSPI1,
  };
  flexspi_config_t flexspi_config;
  FLEXSPI_GetDefaultConfig(&flexspi_config);
  FLEXSPI_Init(FLEXSPI1, &flexspi_config);
  Nand_Flash_Init(&nand_config, &nand_handle);
}

void BOARD_InitHardware(bool init_console) {
  MCMGR_Init();
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_ConfigMPU();
  if (init_console) {
    BOARD_InitDebugConsole();
  }
#if __CORTEX_M == 7
  if (!BOARD_InitSEMC()) {
    printf("Failed to initialize SDRAM.\r\n");
  }

  uint32_t *sdram_rom = &__SDRAM_ROM;
  uint32_t *sdram_bss = &__sdram_bss_start__;
  uint32_t *sdram_data = &__sdram_data_start__;
  uint32_t sdram_data_end = (uint32_t)(&__sdram_data_end__);
  uint32_t sdram_bss_end = (uint32_t)(&__sdram_bss_end__);
  uint32_t sdram_data_start = (uint32_t)(&__sdram_data_start__);
  uint32_t sdram_bss_start = (uint32_t)(&__sdram_bss_start__);
  uint32_t sdram_data_size = sdram_data_end - sdram_data_start;
  uint32_t sdram_bss_size = sdram_bss_end - sdram_bss_start;
  memcpy(sdram_data, sdram_rom, sdram_data_size);
  memset((uint8_t *)sdram_bss, 0x0, sdram_bss_size);

  BOARD_InitCAAM();
#endif  // __CORTEX_M == 7

  BOARD_InitNAND();
}
