/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"

void InitArduinoPins(void);

/* FUNCTION
 * ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END
 * ****************************************************************************************************************/
void BOARD_InitBootPins(void) { BOARD_InitPins(); }

/* FUNCTION
 * ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical
 * features.
 *
 * END
 * ****************************************************************************************************************/
void BOARD_InitPins(void) {
    CLOCK_EnableClock(kCLOCK_Iomuxc); /* LPCG on: LPCG is ON. */

    // TPU MCM Pins
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_16_GPIO8_IO26, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_15_GPIO8_IO25, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_14_GPIO8_IO24, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_13_GPIO8_IO23, 0U);

    // LED pins
    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_03_DIG_GPIO13_IO06, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SNVS_03_DIG_GPIO13_IO06, 0x2U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_02_DIG_GPIO13_IO05, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SNVS_02_DIG_GPIO13_IO05, 0x2U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_02_FLEXPWM1_PWM1_A, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_02_FLEXPWM1_PWM1_A, 0U);

    // User button (pulled-up, input)
    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_00_DIG_GPIO13_IO03, 1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SNVS_00_DIG_GPIO13_IO03, 0xCU);

    // Crypto RST
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_08_GPIO12_IO08, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_LPSR_08_GPIO12_IO08, 0xCU);

    // DMIC
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_00_MIC_CLK, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_LPSR_00_MIC_CLK, 3U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_01_MIC_BITSTREAM0, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_LPSR_01_MIC_BITSTREAM0, 3U);

    // Ethernet
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_00_ENET_1G_RX_EN, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_01_ENET_1G_RX_CLK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_02_ENET_1G_RX_DATA00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_03_ENET_1G_RX_DATA01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_04_ENET_1G_RX_DATA02, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_05_ENET_1G_RX_DATA03, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_06_ENET_1G_TX_DATA03, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_07_ENET_1G_TX_DATA02, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_08_ENET_1G_TX_DATA01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_09_ENET_1G_TX_DATA00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_10_ENET_1G_TX_EN, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B1_11_ENET_1G_TX_CLK_IO, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_19_ENET_1G_MDC, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_20_ENET_1G_MDIO, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_02_GPIO8_IO12, 0U);  // RGMII_PHY_INTB
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_03_GPIO8_IO13, 0U);  // ETHPHY_RST_B
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_05_GPIO9_IO04, 0U);      // Buffer

    IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_B2_02_GPIO8_IO12,
                        0x4U);  // RGMII_PHY_INTB
    IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_B2_03_GPIO8_IO13,
                        0x4U);  // ETHPHY_RST_B
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_05_GPIO9_IO04, 2U);

    // I2C5
    /* From freertos_lpi2c sample */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_LPSR_04_LPI2C5_SDA, /* GPIO_LPSR_04 is configured as
                                           LPI2C5_SDA */
        1U); /* Software Input On Field: Force input path of pad GPIO_LPSR_04 */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_LPSR_05_LPI2C5_SCL, /* GPIO_LPSR_05 is configured as
                                           LPI2C5_SCL */
        1U); /* Software Input On Field: Force input path of pad GPIO_LPSR_05 */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_LPSR_04_LPI2C5_SDA, /* GPIO_LPSR_04 PAD functional
                                           properties : */
        0x20U);                         /* Slew Rate Field: Slow Slew Rate
                                           Drive Strength Field: normal driver
                                           Pull / Keep Select Field: Pull Disable
                                           Pull Up / Down Config. Field: Weak pull down
                                           Open Drain LPSR Field: Enabled
                                           Domain write protection: Both cores are allowed
                                           Domain write protection lock: Neither of DWP bits is locked
                                         */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_LPSR_05_LPI2C5_SCL, /* GPIO_LPSR_05 PAD functional
                                           properties : */
        0x20U);                         /* Slew Rate Field: Slow Slew Rate
                                           Drive Strength Field: normal driver
                                           Pull / Keep Select Field: Pull Disable
                                           Pull Up / Down Config. Field: Weak pull down
                                           Open Drain LPSR Field: Enabled
                                           Domain write protection: Both cores are allowed
                                           Domain write protection lock: Neither of DWP bits is locked
                                         */

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_24_LPUART1_TXD, /* GPIO_AD_24 is configured
                                                       as LPUART1_TXD */
                     0U); /* Software Input On Field: Input Path is determined
                             by functionality */
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_25_LPUART1_RXD, /* GPIO_AD_25 is configured
                                                       as LPUART1_RXD */
                     0U); /* Software Input On Field: Input Path is determined
                             by functionality */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_AD_24_LPUART1_TXD, /* GPIO_AD_24 PAD functional properties :
                                        */
        0x02U);                        /* Slew Rate Field: Slow Slew Rate
                                          Drive Strength Field: high driver
                                          Pull / Keep Select Field: Pull Disable, Highz
                                          Pull Up / Down Config. Field: Weak pull down
                                          Open Drain Field: Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_AD_25_LPUART1_RXD, /* GPIO_AD_25 PAD functional properties :
                                        */
        0x02U);                        /* Slew Rate Field: Slow Slew Rate
                                          Drive Strength Field: high driver
                                          Pull / Keep Select Field: Pull Disable, Highz
                                          Pull Up / Down Config. Field: Weak pull down
                                          Open Drain Field: Disabled */

    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B1_40_LPUART6_TXD, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B1_41_LPUART6_RXD, 0U);

    IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_B1_40_LPUART6_TXD, 0x0EU);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_B1_41_LPUART6_RXD, 0x0EU);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_30_LPUART3_TXD, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_31_LPUART3_RXD, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_30_LPUART3_TXD, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_31_LPUART3_RXD, 0U);

#if __CORTEX_M == 7
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_00_SEMC_DATA00, /* GPIO_EMC_B1_00 is configured as
                                              SEMC_DATA00 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_01_SEMC_DATA01, /* GPIO_EMC_B1_01 is configured as
                                              SEMC_DATA01 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_02_SEMC_DATA02, /* GPIO_EMC_B1_02 is configured as
                                              SEMC_DATA02 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_03_SEMC_DATA03, /* GPIO_EMC_B1_03 is configured as
                                              SEMC_DATA03 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_04_SEMC_DATA04, /* GPIO_EMC_B1_04 is configured as
                                              SEMC_DATA04 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_05_SEMC_DATA05, /* GPIO_EMC_B1_05 is configured as
                                              SEMC_DATA05 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_06_SEMC_DATA06, /* GPIO_EMC_B1_06 is configured as
                                              SEMC_DATA06 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_07_SEMC_DATA07, /* GPIO_EMC_B1_07 is configured as
                                              SEMC_DATA07 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_08_SEMC_DM00, /* GPIO_EMC_B1_08 is configured as
                                            SEMC_DM00 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_09_SEMC_ADDR00, /* GPIO_EMC_B1_09 is configured as
                                              SEMC_ADDR00 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_10_SEMC_ADDR01, /* GPIO_EMC_B1_10 is configured as
                                              SEMC_ADDR01 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_11_SEMC_ADDR02, /* GPIO_EMC_B1_11 is configured as
                                              SEMC_ADDR02 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_12_SEMC_ADDR03, /* GPIO_EMC_B1_12 is configured as
                                              SEMC_ADDR03 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_13_SEMC_ADDR04, /* GPIO_EMC_B1_13 is configured as
                                              SEMC_ADDR04 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_14_SEMC_ADDR05, /* GPIO_EMC_B1_14 is configured as
                                              SEMC_ADDR05 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_15_SEMC_ADDR06, /* GPIO_EMC_B1_15 is configured as
                                              SEMC_ADDR06 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_16_SEMC_ADDR07, /* GPIO_EMC_B1_16 is configured as
                                              SEMC_ADDR07 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_17_SEMC_ADDR08, /* GPIO_EMC_B1_17 is configured as
                                              SEMC_ADDR08 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_18_SEMC_ADDR09, /* GPIO_EMC_B1_18 is configured as
                                              SEMC_ADDR09 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_19_SEMC_ADDR11, /* GPIO_EMC_B1_19 is configured as
                                              SEMC_ADDR11 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_20_SEMC_ADDR12, /* GPIO_EMC_B1_20 is configured as
                                              SEMC_ADDR12 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_21_SEMC_BA0, /* GPIO_EMC_B1_21 is configured as
                                           SEMC_BA0 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_22_SEMC_BA1, /* GPIO_EMC_B1_22 is configured as
                                           SEMC_BA1 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_23_SEMC_ADDR10, /* GPIO_EMC_B1_23 is configured as
                                              SEMC_ADDR10 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_24_SEMC_CAS, /* GPIO_EMC_B1_24 is configured as
                                           SEMC_CAS */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_25_SEMC_RAS, /* GPIO_EMC_B1_25 is configured as
                                           SEMC_RAS */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_26_SEMC_CLK, /* GPIO_EMC_B1_26 is configured as
                                           SEMC_CLK */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_27_SEMC_CKE, /* GPIO_EMC_B1_27 is configured as
                                           SEMC_CKE */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B1_28_SEMC_WE, /* GPIO_EMC_B1_28 is
                                                       configured as SEMC_WE */
                     0U); /* Software Input On Field: Input Path is determined
                             by functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_29_SEMC_CS0, /* GPIO_EMC_B1_29 is configured as
                                           SEMC_CS0 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_30_SEMC_DATA08, /* GPIO_EMC_B1_30 is configured as
                                              SEMC_DATA08 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_31_SEMC_DATA09, /* GPIO_EMC_B1_31 is configured as
                                              SEMC_DATA09 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_32_SEMC_DATA10, /* GPIO_EMC_B1_32 is configured as
                                              SEMC_DATA10 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_33_SEMC_DATA11, /* GPIO_EMC_B1_33 is configured as
                                              SEMC_DATA11 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_34_SEMC_DATA12, /* GPIO_EMC_B1_34 is configured as
                                              SEMC_DATA12 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_35_SEMC_DATA13, /* GPIO_EMC_B1_35 is configured as
                                              SEMC_DATA13 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_36_SEMC_DATA14, /* GPIO_EMC_B1_36 is configured as
                                              SEMC_DATA14 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_37_SEMC_DATA15, /* GPIO_EMC_B1_37 is configured as
                                              SEMC_DATA15 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_38_SEMC_DM01, /* GPIO_EMC_B1_38 is configured as
                                            SEMC_DM01 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B1_39_SEMC_DQS, /* GPIO_EMC_B1_39 is configured as
                                           SEMC_DQS */
        1U); /* Software Input On Field: Force input path of pad GPIO_EMC_B1_39
              */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_EMC_B2_18_SEMC_DQS4, /* GPIO_EMC_B2_18 is configured as
                                            SEMC_DQS4 */
        0U); /* Software Input On Field: Input Path is determined by
                functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B2_06_FLEXSPI1_A_SS0_B, /* GPIO_SD_B2_06 is configured as
                                                  FLEXSPI1_A_SS0_B */
        1U); /* Software Input On Field: Force input path of pad GPIO_SD_B2_06
              */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B2_07_FLEXSPI1_A_SCLK, /* GPIO_SD_B2_07 is configured as
                                                 FLEXSPI1_A_SCLK */
        1U); /* Software Input On Field: Force input path of pad GPIO_SD_B2_07
              */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B2_08_FLEXSPI1_A_DATA00, /* GPIO_SD_B2_08 is configured
                                                   as FLEXSPI1_A_DATA00 */
        1U); /* Software Input On Field: Force input path of pad GPIO_SD_B2_08
              */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B2_09_FLEXSPI1_A_DATA01, /* GPIO_SD_B2_09 is configured
                                                   as FLEXSPI1_A_DATA01 */
        1U); /* Software Input On Field: Force input path of pad GPIO_SD_B2_09
              */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B2_10_FLEXSPI1_A_DATA02, /* GPIO_SD_B2_10 is configured
                                                   as FLEXSPI1_A_DATA02 */
        1U); /* Software Input On Field: Force input path of pad GPIO_SD_B2_10
              */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B2_11_FLEXSPI1_A_DATA03, /* GPIO_SD_B2_11 is configured
                                                   as FLEXSPI1_A_DATA03 */
        1U); /* Software Input On Field: Force input path of pad GPIO_SD_B2_11
              */
    // IOMUXC_SetPinConfig(
    //     IOMUXC_GPIO_SD_B2_05_FLEXSPI1_A_DQS,    /* GPIO_SD_B2_05 PAD
    //     functional properties : */ 0x0AU);                                 /*
    //     PDRV Field: normal driver
    //                                                Pull Down Pull Up Field:
    //                                                PD Open Drain Field:
    //                                                Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B2_06_FLEXSPI1_A_SS0_B, /* GPIO_SD_B2_06 PAD functional
                                                  properties : */
        0x0AU);                                /* PDRV Field: normal driver
                                                  Pull Down Pull Up Field: PD
                                                  Open Drain Field: Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B2_07_FLEXSPI1_A_SCLK, /* GPIO_SD_B2_07 PAD functional
                                                 properties : */
        0x0AU);                               /* PDRV Field: normal driver
                                                 Pull Down Pull Up Field: PD
                                                 Open Drain Field: Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B2_08_FLEXSPI1_A_DATA00, /* GPIO_SD_B2_08 PAD functional
                                                   properties : */
        0x0AU);                                 /* PDRV Field: normal driver
                                                   Pull Down Pull Up Field: PD
                                                   Open Drain Field: Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B2_09_FLEXSPI1_A_DATA01, /* GPIO_SD_B2_09 PAD functional
                                                   properties : */
        0x0AU);                                 /* PDRV Field: normal driver
                                                   Pull Down Pull Up Field: PD
                                                   Open Drain Field: Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B2_10_FLEXSPI1_A_DATA02, /* GPIO_SD_B2_10 PAD functional
                                                   properties : */
        0x0AU);                                 /* PDRV Field: normal driver
                                                   Pull Down Pull Up Field: PD
                                                   Open Drain Field: Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B2_11_FLEXSPI1_A_DATA03, /* GPIO_SD_B2_11 PAD functional
                                                   properties : */
        0x0AU);                                 /* PDRV Field: normal driver
                                                   Pull Down Pull Up Field: PD
                                                   Open Drain Field: Disabled */

    // CAM_INT
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_04_GPIO9_IO03, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_12_GPIO8_IO22, 0U);
    // CAM_TRIG
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_17_GPIO8_IO27, 0U);
    // Camera motion detect
    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_05_DIG_GPIO13_IO08, 0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_14_VIDEO_MUX_CSI_VSYNC, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_15_VIDEO_MUX_CSI_HSYNC, 0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_16_VIDEO_MUX_CSI_DATA09, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_17_VIDEO_MUX_CSI_DATA08, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_18_VIDEO_MUX_CSI_DATA07, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_19_VIDEO_MUX_CSI_DATA06, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_20_VIDEO_MUX_CSI_DATA05, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_21_VIDEO_MUX_CSI_DATA04, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_22_VIDEO_MUX_CSI_DATA03, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_23_VIDEO_MUX_CSI_DATA02, 0U);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_13_VIDEO_MUX_CSI_MCLK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_12_VIDEO_MUX_CSI_PIXCLK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_17_GPIO8_IO27, 0U);

    /* WLAN */
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_00_USDHC1_CMD, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_01_USDHC1_CLK, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_02_USDHC1_DATA0, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_03_USDHC1_DATA1, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_04_USDHC1_DATA2, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 1U);
    // WL_REG_ON
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_34_GPIO10_IO01, 0U);
    // WL_HOST_WAKE
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_07_GPIO11_IO08, 0U);
    // WL_RST#
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_06_GPIO11_IO07, 0U);
    // WIFI_P_EN
    IOMUXC_SetPinMux(IOMUXC_GPIO_SNVS_09_DIG_GPIO13_IO12, 0U);
    // BT_REG_ON
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_35_GPIO10_IO02, 0U);
    // BT_DEV_WAKE
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_14_GPIO11_IO15, 0U);
    // BT_HOST_WAKE
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_15_GPIO11_IO16, 0U);
    // LPUART2 for BT
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_10_LPUART2_TXD, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_11_LPUART2_RXD, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_12_LPUART2_CTS_B, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_DISP_B2_13_LPUART2_RTS_B, 1U);

    // Pull-up
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_00_USDHC1_CMD, 0x4U);
    // no-pull
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_01_USDHC1_CLK, 0xCU);
    // Pull-up
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_02_USDHC1_DATA0, 0x4U);
    // Pull-up
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_03_USDHC1_DATA1, 0x4U);
    // Pull-up
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_04_USDHC1_DATA2, 0x4U);
    // Pull-up
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 0x4U);
    // WL_REG_ON
    // Module has internal pull-down
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_34_GPIO10_IO01, 0U);
    // WL_HOST_WAKE
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_07_GPIO11_IO08, 0x0U);
    // WL_RST#
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_06_GPIO11_IO07, 0U);
    // WIFI_P_EN
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SNVS_09_DIG_GPIO13_IO12, 0x2U);
    // BT_REG_ON
    // Module has internal pull-down
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_35_GPIO10_IO02, 0U);
    // BT_DEV_WAKE
    // Pull-up
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_14_GPIO11_IO15, 0xCU);
    // BT_HOST_WAKE
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_15_GPIO11_IO16, 0xCU);
    // LPUART2 for BT
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_10_LPUART2_TXD, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_11_LPUART2_RXD, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_12_LPUART2_CTS_B, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_DISP_B2_13_LPUART2_RTS_B, 0U);

#endif

    // GPIO Mode for J9/J10 Header
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_09_GPIO_MUX6_IO09, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_06_GPIO_MUX6_IO06, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_32_GPIO_MUX3_IO31, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_33_GPIO_MUX4_IO00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_06_GPIO_MUX3_IO05, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_07_GPIO_MUX3_IO06, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_00_GPIO_MUX2_IO10, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_01_GPIO_MUX2_IO11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_01_GPIO_MUX3_IO00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_00_GPIO_MUX2_IO31, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_07_GPIO_MUX6_IO07, 0U);

    // Start JTAG pins
    // These pins are set to GPIO for the J10 header.
    // To enable JTAG debugging, comment-out the following 3 lines.
    // If LPSR_13, LPSR_14, and LPSR_15 are muxed elsewhere, they
    // must also be left as default or expliclty set to JTAG_MUX.
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_10_GPIO_MUX6_IO10, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_11_GPIO_MUX6_IO11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_12_GPIO_MUX6_IO12, 0U);
    // End JTAG pins


#if defined(CORAL_MICRO_ARDUINO) && (CORAL_MICRO_ARDUINO == 1)
    InitArduinoPins();
#endif
}

#if defined(CORAL_MICRO_ARDUINO) && (CORAL_MICRO_ARDUINO == 1)
void InitArduinoPins(void) {
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_07_GPIO_MUX6_IO07, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_06_GPIO_MUX6_IO06, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_00_GPIO_MUX2_IO10, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_B2_01_GPIO_MUX2_IO11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_00_FLEXPWM1_PWM0_A, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_01_FLEXPWM1_PWM0_B, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_32_LPI2C1_SCL, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_33_LPI2C1_SDA, 1U);

    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_32_LPI2C1_SCL, 0x10);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_33_LPI2C1_SDA, 0x10);

    // SPI
    // SPI is on the same pins as JTAG, comment these out if JTAG
    // needs to do arduino-specific debugging
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_09_LPSPI6_PCS0, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_10_LPSPI6_SCK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_11_LPSPI6_SOUT, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_LPSR_12_LPSPI6_SIN, 0U);

    IOMUXC_SetPinConfig(IOMUXC_GPIO_LPSR_09_LPSPI6_PCS0, 0x0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_LPSR_10_LPSPI6_SCK, 0x0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_LPSR_11_LPSPI6_SOUT, 0x0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_LPSR_12_LPSPI6_SIN, 0x0U);
}
#endif

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
