#include "third_party/modified/nxp/rt1176-sdk/sdmmc_config.h"
#include "third_party/modified/nxp/rt1176-sdk/pin_mux.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"
#include "third_party/nxp/rt1176-sdk/middleware/sdmmc/sdio/fsl_sdio.h"
#include <stdio.h>

#define SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE (32U)
#define SDMMC_HOST_DATA_BUFFER_SIZE (32U)

static sd_detect_card_t s_cd;

void BOARD_SDCardPowerResetInit(void) {
    gpio_pin_config_t wifi_p_en;
    wifi_p_en.direction = kGPIO_DigitalOutput;
    wifi_p_en.outputLogic = 0;
    wifi_p_en.interruptMode = kGPIO_NoIntmode;
    GPIO_PinInit(GPIO13, 12, &wifi_p_en);

    gpio_pin_config_t wl_reg_on;
    wl_reg_on.direction = kGPIO_DigitalOutput;
    wl_reg_on.outputLogic = 0;
    wl_reg_on.interruptMode = kGPIO_NoIntmode;
    GPIO_PinInit(BOARD_INITPINS_WL_REG_ON_GPIO, BOARD_INITPINS_WL_REG_ON_PIN, &wl_reg_on);
}

void BOARD_SDCardPowerControl(bool enable) {
    GPIO_PinWrite(GPIO13, 12, enable);
}

bool BOARD_SDCardGetDetectStatus(void) {
    return true;
}

void BOARD_SDCardDetectInit(sd_cd_t cd, void *userData) {
    s_cd.cdDebounce_ms = 100U;
    s_cd.type = kSD_DetectCardByGpioCD;
    s_cd.cardDetected = BOARD_SDCardGetDetectStatus;
    s_cd.callback = cd;
    s_cd.userData = userData;

    BOARD_SDCardPowerControl(true);
}

#if __CORTEX_M == 7
void BOARD_USDHC_Errata(void)
{
    /* ERR050396
     * Errata description:
     * AXI to AHB conversion for CM7 AHBS port (port to access CM7 to TCM) is by a NIC301 block, instead of XHB400
     * block. NIC301 doesn’t support sparse write conversion. Any AXI to AHB conversion need XHB400, not by NIC. This
     * will result in data corruption in case of AXI sparse write reaches the NIC301 ahead of AHBS. Errata workaround:
     * For uSDHC, don’t set the bit#1 of IOMUXC_GPR28 (AXI transaction is cacheable), if write data to TCM aligned in 4
     * bytes; No such write access limitation for OCRAM or external RAM
     */
    IOMUXC_GPR->GPR28 &= (~IOMUXC_GPR_GPR28_AWCACHE_USDHC_MASK);
}
#endif

uint32_t BOARD_USDHC1ClockConfiguration(void) {
    clock_root_config_t rootCfg = {0};
    const clock_sys_pll2_config_t sysPll2Config = {
        .ssEnable = false,
    };

    CLOCK_InitSysPll2(&sysPll2Config);
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd2, 24);

    rootCfg.mux = 4;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);

    return CLOCK_GetRootClockFreq(kCLOCK_Root_Usdhc1);
}

void BOARD_SDIO_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, sdio_int_t cardInt) {
    AT_NONCACHEABLE_SECTION_ALIGN(static uint32_t s_sdmmcHostDmaBuffer[SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE], SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE);
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    SDK_ALIGN(static uint8_t s_sdmmcCacheLineAlignBuffer[SDMMC_HOST_DATA_BUFFER_SIZE * 2U], SDMMC_HOST_DATA_BUFFER_SIZE);
#endif
    static sdmmchost_t s_host;
    static sd_io_voltage_t s_ioVoltage = {
        .type = kSD_IOVoltageCtrlByHost,
        .func = NULL,
    };
    static sdio_card_int_t s_sdioInt;

    s_host.dmaDesBuffer = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum = SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
#if ((defined __DCACHE_PRESENT) && __DCACHE_PRESENT) || (defined FSL_FEATURE_HAS_L1CACHE && FSL_FEATURE_HAS_L1CACHE)
    s_host.enableCacheControl = kSDMMCHOST_CacheControlRWBuffer;
#endif
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    s_host.cacheAlignBuffer     = s_sdmmcCacheLineAlignBuffer;
    s_host.cacheAlignBufferSize = SDMMC_HOST_DATA_BUFFER_SIZE * 2U;
#endif

    sdio_card_t* sdio_card = (sdio_card_t*)card;
    sdio_card->host = &s_host;
    sdio_card->host->hostController.base = USDHC1;
    sdio_card->host->hostController.sourceClock_Hz = BOARD_USDHC1ClockConfiguration();
    sdio_card->usrParam.cd = &s_cd;
    sdio_card->usrParam.pwr = BOARD_SDCardPowerControl;
    sdio_card->usrParam.ioStrength = NULL;
    sdio_card->usrParam.ioVoltage = &s_ioVoltage;
    sdio_card->usrParam.maxFreq = 200000000U;

    if (cardInt) {
        s_sdioInt.cardInterrupt = cardInt;
        sdio_card->usrParam.sdioInt = &s_sdioInt;
    }

    BOARD_SDCardPowerResetInit();
    BOARD_SDCardDetectInit(cd, NULL);

    NVIC_SetPriority(USDHC1_IRQn, hostIRQPriority);

#if __CORTEX_M == 7
    BOARD_USDHC_Errata();
#endif

}
