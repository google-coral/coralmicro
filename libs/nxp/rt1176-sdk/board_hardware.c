#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/peripherals.h"
#include "libs/nxp/rt1176-sdk/pin_mux.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_semc.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"

#include <stdio.h>
#include <string.h>

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

void SystemInitHook(void) {
    MCMGR_EarlyInit();
}

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
    sdramconfig.csxPinMux           = kSEMC_MUXCSX0;
    sdramconfig.address             = 0x80000000;
    sdramconfig.memsize_kbytes      = 32 * 1024; /* 32MB = 32*1024*1KBytes*/
    sdramconfig.portSize            = kSEMC_PortSize16Bit;
    sdramconfig.burstLen            = kSEMC_Sdram_BurstLen8;
    sdramconfig.columnAddrBitNum    = kSEMC_SdramColunm_9bit;
    sdramconfig.casLatency          = kSEMC_LatencyThree;
    sdramconfig.tPrecharge2Act_Ns   = 15; /* tRP 15ns */
    sdramconfig.tAct2ReadWrite_Ns   = 15; /* tRCD 15ns */
    sdramconfig.tRefreshRecovery_Ns = 70; /* Use the maximum of the (Trfc , Txsr). */
    sdramconfig.tWriteRecovery_Ns   = 2;  /* tWR 2ns */
    sdramconfig.tCkeOff_Ns =
        42; /* The minimum cycle of SDRAM CLK off state. CKE is off in self refresh at a minimum period tRAS.*/
    sdramconfig.tAct2Prechage_Ns       = 40; /* tRAS 40ns */
    sdramconfig.tSelfRefRecovery_Ns    = 70;
    sdramconfig.tRefresh2Refresh_Ns    = 60;
    sdramconfig.tAct2Act_Ns            = 2; /* tRC/tRDD 2ns */
    sdramconfig.tPrescalePeriod_Ns     = 160 * (1000000000 / clockFrq);
    sdramconfig.refreshPeriod_nsPerRow = 64 * 1000000 / 8192; /* 64ms/8192 */
    sdramconfig.refreshUrgThreshold    = sdramconfig.refreshPeriod_nsPerRow;
    sdramconfig.refreshBurstLen        = 1;
    sdramconfig.delayChain             = 2;

    return SEMC_ConfigureSDRAM(SEMC, kSEMC_SDRAM_CS0, &sdramconfig, clockFrq) == kStatus_Success;
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

void BOARD_InitHardware(void) {
    MCMGR_Init();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#if __CORTEX_M == 7
    BOARD_InitDebugConsole();
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
    memset((uint8_t*)sdram_bss, 0x0, sdram_bss_size);

    BOARD_InitCAAM();
#endif
}
