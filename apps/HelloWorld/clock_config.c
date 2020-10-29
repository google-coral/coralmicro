/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "fsl_iomuxc.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockRUN ***********************
 ******************************************************************************/

#ifndef SKIP_DCDC_ADJUSTMENT
#if __CORTEX_M == 4
#define SKIP_DCDC_ADJUSTMENT 1
#endif
#endif

/* 1.0 v */
#define DCDC_TARGET_VOLTAGE_1V 1000
/* 1.15 v */
#define DCDC_TARGET_VOLTAGE_1P15V 1150

#ifndef DCDC_TARGET_VOLTAGE
#define DCDC_TARGET_VOLTAGE DCDC_TARGET_VOLTAGE_1P15V
#endif

/*
 * TODO:
 *   replace the following two dcdc function with SDK DCDC driver when it's
 * ready
 */
/*******************************************************************************
 * Code for getting current DCDC voltage setting
 ******************************************************************************/
uint32_t dcdc_get_target_voltage()
{
    uint32_t temp = DCDC->CTRL1;
    temp          = (temp & DCDC_CTRL1_VDD1P0CTRL_TRG_MASK) >> DCDC_CTRL1_VDD1P0CTRL_TRG_SHIFT;
    return (temp * 25 + 600);
}

/*******************************************************************************
 * Code for setting DCDC to target voltage
 ******************************************************************************/
void dcdc_trim_target_1p0(uint32_t target_voltage)
{
    uint8_t trim_value;
    uint32_t temp;

    trim_value = (target_voltage - 600) / 25;
    temp       = DCDC->CTRL1;
    if ((temp & DCDC_CTRL1_VDD1P0CTRL_TRG(trim_value)) == DCDC_CTRL1_VDD1P0CTRL_TRG(trim_value))
    {
        return;
    }

    temp &= ~DCDC_CTRL1_VDD1P0CTRL_TRG_MASK;
    temp |= DCDC_CTRL1_VDD1P0CTRL_TRG(trim_value);
    DCDC->CTRL1 = temp;
}

/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
    clock_root_config_t rootCfg = {0};

#if !defined(SKIP_DCDC_ADJUSTMENT) || (!SKIP_DCDC_ADJUSTMENT)
    dcdc_trim_target_1p0(DCDC_TARGET_VOLTAGE);
#endif

#if defined(BYPASS_LDO_LPSR) && BYPASS_LDO_LPSR
    CLOCK_ANATOP_LdoLpsrAnaBypassOn();
    CLOCK_ANATOP_LdoLpsrDigBypassOn();
#endif

#if __CORTEX_M == 7
    /* ARM PLL 996 MHz. */
    const clock_arm_pll_config_t armPllConfig = {
        .postDivider = kCLOCK_PllPostDiv2,
        .loopDivider = 166,
    };
#endif

    /* SYS PLL2 528MHz. */
    const clock_sys_pll_config_t sysPllConfig = {
        .loopDivider = 1,
        /* Using 24Mhz OSC */
        .mfn = 0,
        .mfi = 22,
    };

    const clock_sys_pll3_config_t sysPll3Config = {
        .divSelect = 3,
    };

    /* PLL LDO shall be enabled first before enable PLLs */
    CLOCK_EnableOsc24M();

#if __CORTEX_M == 7
    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_M7_Systick, &rootCfg);

    /* ARMPll: 996M */
    CLOCK_InitArmPll(&armPllConfig);
    /* Configure M7 */
    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxArmPllOut;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);

    /* Configure M7 Systick running at 10K */
    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 239;
    CLOCK_SetRootClock(kCLOCK_Root_M7_Systick, &rootCfg);
#endif
    CLOCK_InitSysPll2(&sysPllConfig);
    CLOCK_InitSysPll3(&sysPll3Config);

#if __CORTEX_M == 4
    rootCfg.mux = kCLOCK_M4_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);

    CLOCK_InitPfd(kCLOCK_PllSys3, kCLOCK_Pfd3, 22);
    /* Configure M4 using SysPll3Pfd3 divided by 1 */
    rootCfg.mux = kCLOCK_M4_ClockRoot_MuxSysPll3Pfd3;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);

    /* SysPll3 divide by 4 */
    rootCfg.mux = kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 3;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);
#endif

#if DEBUG_CONSOLE_UART_INDEX == 1
    /* Configure Lpuart1 using SysPll2*/
    rootCfg.mux = kCLOCK_LPUART1_ClockRoot_MuxSysPll2Out;
    rootCfg.div = 21;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart1, &rootCfg);
#else
    /* Configure Lpuart2 using SysPll2*/
    rootCfg.mux = kCLOCK_LPUART2_ClockRoot_MuxSysPll2Out;
    rootCfg.div = 21;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart2, &rootCfg);
#endif

#ifndef SKIP_SEMC_INIT
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd1, 16);
    /* Configure Semc using SysPll2Pfd1 divided by 3 */
    rootCfg.mux = kCLOCK_SEMC_ClockRoot_MuxSysPll2Pfd1;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Semc, &rootCfg);
#endif

    /* Configure Bus using SysPll3 divided by 4 */
    rootCfg.mux = kCLOCK_BUS_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg);

    /* Configure Lpi2c1 using Osc48MDiv2 */
    rootCfg.mux = kCLOCK_LPI2C1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c1, &rootCfg);

    /* Configure Lpi2c5 using Osc48MDiv2 */
    rootCfg.mux = kCLOCK_LPI2C5_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c5, &rootCfg);

    /* Configure gpt timer using Osc48MDiv2 */
    rootCfg.mux = kCLOCK_GPT1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt1, &rootCfg);

    /* Configure gpt timer using Osc48MDiv2 */
    rootCfg.mux = kCLOCK_GPT2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt2, &rootCfg);

    /* Configure lpspi using Osc48MDiv2 */
    rootCfg.mux = kCLOCK_LPSPI1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Lpspi1, &rootCfg);

    /* Configure flexio using Osc48MDiv2 */
    rootCfg.mux = kCLOCK_FLEXIO2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Flexio2, &rootCfg);

    /* Configure emvsim using Osc48MDiv2 */
    rootCfg.mux = kCLOCK_EMV1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 0;
    CLOCK_SetRootClock(kCLOCK_Root_Emv1, &rootCfg);

#if __CORTEX_M == 7
    SystemCoreClock = CLOCK_GetRootClockFreq(kCLOCK_Root_M7);
#else
    SystemCoreClock = CLOCK_GetRootClockFreq(kCLOCK_Root_M4);
#endif
}
