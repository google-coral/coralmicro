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

/*
 * How to setup clock using clock driver functions:
 *
 * 1. Call CLOCK_InitXXXPLL() to configure corresponding PLL clock.
 *
 * 2. Call CLOCK_InitXXXpfd() to configure corresponding PLL pfd clock.
 *
 * 3. Call CLOCK_SetRootClock() to configure corresponding module clock source and divider.
 *
 */

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Clocks v9.0
processor: MIMXRT1176xxxxx
package_id: MIMXRT1176CVM8A
mcu_data: ksdk2_0
processor_version: 11.0.1
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

#include "clock_config.h"
#include "fsl_iomuxc.h"
#include "fsl_dcdc.h"
#include "fsl_pmu.h"
#include "fsl_clock.h"

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
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!Configuration
name: BOARD_BootClockRUN
called_from_default_init: true
outputs:
- {id: ARM_PLL_CLK.outFreq, value: 792 MHz, locked: true, accuracy: '0.001'}
- {id: BUS_CLK_ROOT.outFreq, value: 240 MHz}
- {id: BUS_LPSR_CLK_ROOT.outFreq, value: 160 MHz, locked: true, accuracy: '0.001'}
- {id: CLK_1M.outFreq, value: 1 MHz}
- {id: CSI_CLK_ROOT.outFreq, value: 24 MHz}
- {id: CSSYS_CLK_ROOT.outFreq, value: 24 MHz}
- {id: CSTRACE_CLK_ROOT.outFreq, value: 132 MHz, locked: true, accuracy: '0.001'}
- {id: ENET2_CLK_ROOT.outFreq, value: 125 MHz, locked: true, accuracy: '0.001'}
- {id: ENET_1G_TX_CLK.outFreq, value: 125 MHz}
- {id: ENET_25M_CLK_ROOT.outFreq, value: 24 MHz}
- {id: ENET_QOS_CLK_ROOT.outFreq, value: 24 MHz}
- {id: ENET_TIMER1_CLK_ROOT.outFreq, value: 24 MHz}
- {id: ENET_TIMER2_CLK_ROOT.outFreq, value: 24 MHz}
- {id: ENET_TIMER3_CLK_ROOT.outFreq, value: 24 MHz}
- {id: FLEXSPI1_CLK_ROOT.outFreq, value: 24 MHz}
- {id: FLEXSPI2_CLK_ROOT.outFreq, value: 24 MHz}
- {id: GPT1_CLK_ROOT.outFreq, value: 24 MHz}
- {id: GPT1_ipg_clk_highfreq.outFreq, value: 24 MHz}
- {id: GPT2_CLK_ROOT.outFreq, value: 1 MHz}
- {id: GPT2_ipg_clk_highfreq.outFreq, value: 1 MHz}
- {id: GPT3_CLK_ROOT.outFreq, value: 1 MHz}
- {id: GPT3_ipg_clk_highfreq.outFreq, value: 1 MHz}
- {id: GPT4_CLK_ROOT.outFreq, value: 24 MHz}
- {id: GPT4_ipg_clk_highfreq.outFreq, value: 24 MHz}
- {id: GPT5_CLK_ROOT.outFreq, value: 24 MHz}
- {id: GPT5_ipg_clk_highfreq.outFreq, value: 24 MHz}
- {id: GPT6_CLK_ROOT.outFreq, value: 24 MHz}
- {id: GPT6_ipg_clk_highfreq.outFreq, value: 24 MHz}
- {id: LPI2C1_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPI2C2_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPI2C3_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPI2C4_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPI2C5_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPI2C6_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPSPI1_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPSPI2_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPSPI3_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPSPI4_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPSPI5_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPSPI6_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART10_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART11_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART12_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART1_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART2_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART3_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART4_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART5_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART6_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART7_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART8_CLK_ROOT.outFreq, value: 24 MHz}
- {id: LPUART9_CLK_ROOT.outFreq, value: 24 MHz}
- {id: M4_CLK_ROOT.outFreq, value: 392.72 MHz, locked: true, accuracy: '0.001'}
- {id: M4_SYSTICK_CLK_ROOT.outFreq, value: 100 kHz, locked: true, accuracy: '0.001'}
- {id: M7_CLK_ROOT.outFreq, value: 792 MHz, locked: true, accuracy: '0.001'}
- {id: M7_SYSTICK_CLK_ROOT.outFreq, value: 100 kHz}
- {id: MIC_CLK_ROOT.outFreq, value: 24.57 MHz, locked: true, accuracy: '0.001'}
- {id: MQS_CLK_ROOT.outFreq, value: 24 MHz}
- {id: MQS_MCLK.outFreq, value: 24 MHz}
- {id: OSC_24M.outFreq, value: 24 MHz}
- {id: OSC_32K.outFreq, value: 32.768 kHz}
- {id: OSC_RC_16M.outFreq, value: 16 MHz}
- {id: OSC_RC_400M.outFreq, value: 400 MHz}
- {id: OSC_RC_48M.outFreq, value: 48 MHz}
- {id: OSC_RC_48M_DIV2.outFreq, value: 24 MHz}
- {id: PLL_AUDIO_CLK.outFreq, value: 49.155 MHz}
- {id: SAI1_CLK_ROOT.outFreq, value: 24 MHz}
- {id: SAI1_MCLK1.outFreq, value: 24 MHz}
- {id: SAI2_CLK_ROOT.outFreq, value: 24 MHz}
- {id: SAI2_MCLK1.outFreq, value: 24 MHz}
- {id: SAI3_CLK_ROOT.outFreq, value: 24 MHz}
- {id: SAI3_MCLK1.outFreq, value: 24 MHz}
- {id: SAI4_CLK_ROOT.outFreq, value: 24 MHz}
- {id: SAI4_MCLK1.outFreq, value: 24 MHz}
- {id: SEMC_CLK_ROOT.outFreq, value: 166.73 MHz, locked: true, accuracy: '0.001'}
- {id: SYS_PLL1_CLK.outFreq, value: 1 GHz}
- {id: SYS_PLL1_DIV2_CLK.outFreq, value: 500 MHz}
- {id: SYS_PLL2_CLK.outFreq, value: 528 MHz}
- {id: SYS_PLL2_PFD0_CLK.outFreq, value: 352 MHz}
- {id: SYS_PLL2_PFD1_CLK.outFreq, value: 9504/19 MHz}
- {id: SYS_PLL2_PFD2_CLK.outFreq, value: 396 MHz}
- {id: SYS_PLL2_PFD3_CLK.outFreq, value: 297 MHz}
- {id: SYS_PLL3_CLK.outFreq, value: 480 MHz}
- {id: SYS_PLL3_PFD0_CLK.outFreq, value: 8640/13 MHz}
- {id: SYS_PLL3_PFD1_CLK.outFreq, value: 8640/17 MHz}
- {id: SYS_PLL3_PFD2_CLK.outFreq, value: 270 MHz}
- {id: SYS_PLL3_PFD3_CLK.outFreq, value: 4320/11 MHz}
- {id: USDHC1_CLK_ROOT.outFreq, value: 24 MHz}
settings:
- {id: CoreBusClockRootsInitializationConfig, value: selectedCore}
- {id: SOCDomainVoltage, value: OD}
- {id: SemcConfigurationPatchConfig, value: disabled}
- {id: ANADIG_OSC_OSC_24M_CTRL_OSC_EN_CFG, value: Enabled}
- {id: ANADIG_PLL.ARM_PLL_POST_DIV.scale, value: '2', locked: true}
- {id: ANADIG_PLL.ARM_PLL_PREDIV.scale, value: '1', locked: true}
- {id: ANADIG_PLL.ARM_PLL_VDIV.scale, value: '66', locked: true}
- {id: ANADIG_PLL.PLL_AUDIO.denom, value: '100'}
- {id: ANADIG_PLL.PLL_AUDIO.div, value: '32'}
- {id: ANADIG_PLL.PLL_AUDIO.num, value: '77'}
- {id: ANADIG_PLL.PLL_AUDIO_POST_DIV.scale, value: '16', locked: true}
- {id: ANADIG_PLL.PLL_AUDIO_SS_DIV.scale, value: '100'}
- {id: ANADIG_PLL.PLL_VIDEO_BYPASS.sel, value: ANADIG_OSC.OSC_24M}
- {id: ANADIG_PLL.SYS_PLL2.denom, value: '1'}
- {id: ANADIG_PLL.SYS_PLL2.div, value: '22'}
- {id: ANADIG_PLL.SYS_PLL2.num, value: '0'}
- {id: ANADIG_PLL.SYS_PLL2_PFD1_DIV.scale, value: '19', locked: true}
- {id: ANADIG_PLL.SYS_PLL2_PFD1_MUL.scale, value: '18', locked: true}
- {id: ANADIG_PLL.SYS_PLL2_SS_DIV.scale, value: '1'}
- {id: ANADIG_PLL.SYS_PLL3_PFD3_DIV.scale, value: '22', locked: true}
- {id: ANADIG_PLL.SYS_PLL3_PFD3_MUL.scale, value: '18', locked: true}
- {id: ANADIG_PLL_ARM_PLL_CTRL_POWERUP_CFG, value: Enabled}
- {id: ANADIG_PLL_PLL_AUDIO_CTRL0_POWERUP_CFG, value: Enabled}
- {id: ANADIG_PLL_PLL_VIDEO_CTRL_GATE_CFG, value: Disabled}
- {id: ANADIG_PLL_SYS_PLL1_CTRL_SYS_PLL1_DIV2_CFG, value: Enabled}
- {id: ANADIG_PLL_SYS_PLL2_CTRL_POWERUP_CFG, value: Enabled}
- {id: ANADIG_PLL_SYS_PLL3_CTRL_POWERUP_CFG, value: Enabled}
- {id: CCM.CLOCK_ROOT0.DIV.scale, value: '1', locked: true}
- {id: CCM.CLOCK_ROOT0.MUX.sel, value: ANADIG_PLL.ARM_PLL_CLK}
- {id: CCM.CLOCK_ROOT1.DIV.scale, value: '1', locked: true}
- {id: CCM.CLOCK_ROOT1.MUX.sel, value: ANADIG_PLL.SYS_PLL3_PFD3_CLK}
- {id: CCM.CLOCK_ROOT2.DIV.scale, value: '2', locked: true}
- {id: CCM.CLOCK_ROOT2.MUX.sel, value: ANADIG_PLL.SYS_PLL3_CLK}
- {id: CCM.CLOCK_ROOT3.DIV.scale, value: '3', locked: true}
- {id: CCM.CLOCK_ROOT3.MUX.sel, value: ANADIG_PLL.SYS_PLL3_CLK}
- {id: CCM.CLOCK_ROOT4.DIV.scale, value: '3', locked: true}
- {id: CCM.CLOCK_ROOT4.MUX.sel, value: ANADIG_PLL.SYS_PLL2_PFD1_CLK}
- {id: CCM.CLOCK_ROOT52.DIV.scale, value: '4'}
- {id: CCM.CLOCK_ROOT52.MUX.sel, value: ANADIG_PLL.SYS_PLL1_DIV2_CLK}
- {id: CCM.CLOCK_ROOT6.DIV.scale, value: '4'}
- {id: CCM.CLOCK_ROOT6.MUX.sel, value: ANADIG_PLL.SYS_PLL2_CLK}
- {id: CCM.CLOCK_ROOT62.DIV.scale, value: '2', locked: true}
- {id: CCM.CLOCK_ROOT62.MUX.sel, value: ANADIG_PLL.PLL_AUDIO_CLK}
- {id: CCM.CLOCK_ROOT7.DIV.scale, value: '240'}
- {id: CCM.CLOCK_ROOT8.DIV.scale, value: '240', locked: true}
- {id: CLK_ROOT10_INIT_Config, value: disabled}
- {id: CLK_ROOT11_INIT_Config, value: disabled}
- {id: CLK_ROOT12_INIT_Config, value: disabled}
- {id: CLK_ROOT13_INIT_Config, value: disabled}
- {id: CLK_ROOT21_INIT_Config, value: customInit}
- {id: CLK_ROOT22_INIT_Config, value: disabled}
- {id: CLK_ROOT23_INIT_Config, value: disabled}
- {id: CLK_ROOT24_INIT_Config, value: disabled}
- {id: CLK_ROOT49_INIT_Config, value: disabled}
- {id: CLK_ROOT50_INIT_Config, value: disabled}
- {id: CLK_ROOT51_INIT_Config, value: disabled}
- {id: CLK_ROOT59_INIT_Config, value: disabled}
- {id: CLK_ROOT60_INIT_Config, value: disabled}
- {id: CLK_ROOT63_INIT_Config, value: disabled}
- {id: CLK_ROOT68_INIT_Config, value: disabled}
- {id: CLK_ROOT69_INIT_Config, value: disabled}
- {id: CLK_ROOT70_INIT_Config, value: disabled}
- {id: CLK_ROOT71_INIT_Config, value: disabled}
- {id: CLK_ROOT72_INIT_Config, value: disabled}
- {id: CLK_ROOT73_INIT_Config, value: disabled}
- {id: CLK_ROOT74_INIT_Config, value: disabled}
- {id: CLK_ROOT75_INIT_Config, value: disabled}
- {id: CLK_ROOT77_INIT_Config, value: disabled}
- {id: CLK_ROOT78_INIT_Config, value: disabled}
- {id: CLK_ROOT9_INIT_Config, value: disabled}
- {id: CLOCK_GROUP0_CONTROL_OFF_CFG, value: 'no'}
- {id: CLOCK_GROUP1_CONTROL_OFF_CFG, value: 'no'}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/

#ifndef SKIP_POWER_ADJUSTMENT
#if __CORTEX_M == 7
#define BYPASS_LDO_LPSR 1
#define SKIP_LDO_ADJUSTMENT 1
#elif __CORTEX_M == 4
#define SKIP_DCDC_ADJUSTMENT 1
#define SKIP_FBB_ENABLE 1
#endif
#endif

const clock_arm_pll_config_t armPllConfig_BOARD_BootClockRUN =
    {
        .postDivider = kCLOCK_PllPostDiv2,        /* Post divider, 0 - DIV by 2, 1 - DIV by 4, 2 - DIV by 8, 3 - DIV by 1 */
        .loopDivider = 132,                       /* PLL Loop divider, Fout = Fin * ( loopDivider / ( 2 * postDivider ) ) */
    };

const clock_sys_pll1_config_t sysPll1Config_BOARD_BootClockRUN =
    {
        .pllDiv2En = 1,                           /* Enable Sys Pll1 divide-by-2 clock or not */
        .pllDiv5En = 0,                           /* Enable Sys Pll1 divide-by-5 clock or not */
        .ss = NULL,                               /* Spread spectrum parameter */
        .ssEnable = false,                        /* Enable spread spectrum or not */
    };

const clock_sys_pll2_config_t sysPll2Config_BOARD_BootClockRUN =
    {
        .mfd = 1,                                 /* Denominator of spread spectrum */
        .ss = NULL,                               /* Spread spectrum parameter */
        .ssEnable = false,                        /* Enable spread spectrum or not */
    };

const clock_audio_pll_config_t audioPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 32,                        /* PLL Loop divider, valid range for DIV_SELECT divider value: 27 ~ 54. */
        .postDivider = 4,                         /* Divider after PLL, should only be 1, 2, 4, 8, 16, 32 */
        .numerator = 77,                          /* 30 bit numerator of fractional loop divider, Fout = Fin * ( loopDivider + numerator / denominator ) */
        .denominator = 100,                       /* 30 bit denominator of fractional loop divider, Fout = Fin * ( loopDivider + numerator / denominator ) */
        .ss = NULL,                               /* Spread spectrum parameter */
        .ssEnable = false,                        /* Enable spread spectrum or not */
    };

/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
    clock_root_config_t rootCfg = {0};

    /* Set DCDC to DCM mode to improve the efficiency for light loading in run mode and transient performance with a big loading step. */
    DCDC_BootIntoDCM(DCDC);

#if !defined(SKIP_DCDC_ADJUSTMENT) || (!SKIP_DCDC_ADJUSTMENT)
    if((OCOTP->FUSEN[16].FUSE == 0x57AC5969U) && ((OCOTP->FUSEN[17].FUSE & 0xFFU) == 0x0BU))
    {
        DCDC_SetVDD1P0BuckModeTargetVoltage(DCDC, kDCDC_1P0BuckTarget1P15V);
    }
    else
    {
        /* Set 1.125V for production samples to align with data sheet requirement */
        DCDC_SetVDD1P0BuckModeTargetVoltage(DCDC, kDCDC_1P0BuckTarget1P125V);
    }
#endif

#if !defined(SKIP_FBB_ENABLE) || (!SKIP_FBB_ENABLE)
    /* Check if FBB need to be enabled in OverDrive(OD) mode */
    if(((OCOTP->FUSEN[7].FUSE & 0x10U) >> 4U) != 1)
    {
        PMU_EnableBodyBias(ANADIG_PMU, kPMU_FBB_CM7, true);
    }
    else
    {
        PMU_EnableBodyBias(ANADIG_PMU, kPMU_FBB_CM7, false);
    }
#endif

#if defined(BYPASS_LDO_LPSR) && BYPASS_LDO_LPSR
    PMU_StaticEnableLpsrAnaLdoBypassMode(ANADIG_LDO_SNVS, true);
    PMU_StaticEnableLpsrDigLdoBypassMode(ANADIG_LDO_SNVS, true);
#endif

#if !defined(SKIP_LDO_ADJUSTMENT) || (!SKIP_LDO_ADJUSTMENT)
    pmu_static_lpsr_ana_ldo_config_t lpsrAnaConfig;
    pmu_static_lpsr_dig_config_t lpsrDigConfig;

    if((ANADIG_LDO_SNVS->PMU_LDO_LPSR_ANA & ANADIG_LDO_SNVS_PMU_LDO_LPSR_ANA_BYPASS_MODE_EN_MASK) == 0UL)
    {
        PMU_StaticGetLpsrAnaLdoDefaultConfig(&lpsrAnaConfig);
        PMU_StaticLpsrAnaLdoInit(ANADIG_LDO_SNVS, &lpsrAnaConfig);
    }

    if((ANADIG_LDO_SNVS->PMU_LDO_LPSR_DIG & ANADIG_LDO_SNVS_PMU_LDO_LPSR_DIG_BYPASS_MODE_MASK) == 0UL)
    {
        PMU_StaticGetLpsrDigLdoDefaultConfig(&lpsrDigConfig);
        lpsrDigConfig.targetVoltage = kPMU_LpsrDigTargetStableVoltage1P117V;
        PMU_StaticLpsrDigLdoInit(ANADIG_LDO_SNVS, &lpsrDigConfig);
    }
#endif

    /* Config CLK_1M */
    CLOCK_OSC_Set1MHzOutputBehavior(kCLOCK_1MHzOutEnableFreeRunning1Mhz);

    /* Init OSC RC 16M */
    ANADIG_OSC->OSC_16M_CTRL |= ANADIG_OSC_OSC_16M_CTRL_EN_IRC4M16M_MASK;

    /* Init OSC RC 400M */
    CLOCK_OSC_EnableOscRc400M();
    CLOCK_OSC_GateOscRc400M(true);

    /* Init OSC RC 48M */
    CLOCK_OSC_EnableOsc48M(true);
    CLOCK_OSC_EnableOsc48MDiv2(true);

    /* Config OSC 24M */
    ANADIG_OSC->OSC_24M_CTRL |= ANADIG_OSC_OSC_24M_CTRL_OSC_EN(1) | ANADIG_OSC_OSC_24M_CTRL_BYPASS_EN(0) | ANADIG_OSC_OSC_24M_CTRL_BYPASS_CLK(0) | ANADIG_OSC_OSC_24M_CTRL_LP_EN(0) | ANADIG_OSC_OSC_24M_CTRL_OSC_24M_GATE(0);
    /* Wait for 24M OSC to be stable. */
    while (ANADIG_OSC_OSC_24M_CTRL_OSC_24M_STABLE_MASK !=
            (ANADIG_OSC->OSC_24M_CTRL & ANADIG_OSC_OSC_24M_CTRL_OSC_24M_STABLE_MASK))
    {
    }

    /* Swicth both core, M7 Systick and Bus_Lpsr to OscRC48MDiv2 first */
#if __CORTEX_M == 7
    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);

    rootCfg.mux = kCLOCK_M7_SYSTICK_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7_Systick, &rootCfg);
#endif
#if __CORTEX_M == 4
    rootCfg.mux = kCLOCK_M4_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);

    rootCfg.mux = kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);
#endif

    /* Init Arm Pll. */
    CLOCK_InitArmPll(&armPllConfig_BOARD_BootClockRUN);

    /* Init Sys Pll1. */
    CLOCK_InitSysPll1(&sysPll1Config_BOARD_BootClockRUN);

    /* Init Sys Pll2. */
    CLOCK_InitSysPll2(&sysPll2Config_BOARD_BootClockRUN);

    /* Init System Pll2 pfd0. */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd0, 27);

    /* Init System Pll2 pfd1. */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd1, 19);

    /* Init System Pll2 pfd2. */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd2, 24);

    /* Init System Pll2 pfd3. */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 32);

    /* Init Sys Pll3. */
    CLOCK_InitSysPll3();

    /* Init System Pll3 pfd0. */
    CLOCK_InitPfd(kCLOCK_PllSys3, kCLOCK_Pfd0, 13);

    /* Init System Pll3 pfd1. */
    CLOCK_InitPfd(kCLOCK_PllSys3, kCLOCK_Pfd1, 17);

    /* Init System Pll3 pfd2. */
    CLOCK_InitPfd(kCLOCK_PllSys3, kCLOCK_Pfd2, 32);

    /* Init System Pll3 pfd3. */
    CLOCK_InitPfd(kCLOCK_PllSys3, kCLOCK_Pfd3, 22);

    /* Disable Sys Pll3 Div2 output. */
    ANADIG_PLL->SYS_PLL3_CTRL &= ~ANADIG_PLL_SYS_PLL3_CTRL_SYS_PLL3_DIV2_MASK;

    /* Init Audio Pll. */
    CLOCK_InitAudioPll(&audioPllConfig_BOARD_BootClockRUN);

    /* Bypass Video Pll. */
    CLOCK_SetPllBypass(kCLOCK_PllVideo, true);

    /* DeInit Video Pll. */
    CLOCK_DeinitVideoPll();

    /* Module clock root configurations. */
    /* Configure M7 using ARM_PLL_CLK */
#if __CORTEX_M == 7
    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxArmPllOut;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
#endif

    /* Configure M4 using SYS_PLL3_PFD3_CLK */
#if __CORTEX_M == 4
    rootCfg.mux = kCLOCK_M4_ClockRoot_MuxSysPll3Pfd3;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);
#endif

    /* Configure BUS using SYS_PLL3_CLK */
#if __CORTEX_M == 7
    rootCfg.mux = kCLOCK_BUS_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg);
#endif

    /* Configure BUS_LPSR using SYS_PLL3_CLK */
#if __CORTEX_M == 4
    rootCfg.mux = kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 3;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);
#endif

    /* Configure SEMC using SYS_PLL2_PFD1_CLK */
#ifndef SKIP_SEMC_INIT
    rootCfg.mux = kCLOCK_SEMC_ClockRoot_MuxSysPll2Pfd1;
    rootCfg.div = 3;
    CLOCK_SetRootClock(kCLOCK_Root_Semc, &rootCfg);
#endif

    /* Configure CSSYS using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_CSSYS_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Cssys, &rootCfg);

    /* Configure CSTRACE using SYS_PLL2_CLK */
    rootCfg.mux = kCLOCK_CSTRACE_ClockRoot_MuxSysPll2Out;
    rootCfg.div = 4;
    CLOCK_SetRootClock(kCLOCK_Root_Cstrace, &rootCfg);

    /* Configure M4_SYSTICK using OSC_RC_48M_DIV2 */
#if __CORTEX_M == 4
    rootCfg.mux = kCLOCK_M4_SYSTICK_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 240;
    CLOCK_SetRootClock(kCLOCK_Root_M4_Systick, &rootCfg);
#endif

    /* Configure M7_SYSTICK using OSC_RC_48M_DIV2 */
#if __CORTEX_M == 7
    rootCfg.mux = kCLOCK_M7_SYSTICK_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 240;
    CLOCK_SetRootClock(kCLOCK_Root_M7_Systick, &rootCfg);
#endif

    /* Configure GPT1 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_GPT1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt1, &rootCfg);

    /* Configure GPT2 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_GPT2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 24;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt2, &rootCfg);

    /* Configure GPT3 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_GPT3_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 24;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt3, &rootCfg);

    /* Configure GPT4 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_GPT4_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt4, &rootCfg);

    /* Configure GPT5 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_GPT5_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt5, &rootCfg);

    /* Configure GPT6 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_GPT6_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Gpt6, &rootCfg);

    /* Configure FLEXSPI1 using OSC_RC_48M_DIV2 */
#if !(defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1) || defined(FLEXSPI_IN_USE))
    rootCfg.mux = kCLOCK_FLEXSPI1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Flexspi1, &rootCfg);
#endif

    /* Configure FLEXSPI2 using OSC_RC_48M_DIV2 */
    /* CUSTOM INITIALIZATION CODE SHALL BE PROVIDED */

    /* Configure LPUART1 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart1, &rootCfg);

    /* Configure LPUART2 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart2, &rootCfg);

    /* Configure LPUART3 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART3_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart3, &rootCfg);

    /* Configure LPUART4 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART4_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart4, &rootCfg);

    /* Configure LPUART5 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART5_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart5, &rootCfg);

    /* Configure LPUART6 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART6_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart6, &rootCfg);

    /* Configure LPUART7 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART7_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart7, &rootCfg);

    /* Configure LPUART8 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART8_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart8, &rootCfg);

    /* Configure LPUART9 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART9_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart9, &rootCfg);

    /* Configure LPUART10 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART10_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart10, &rootCfg);

    /* Configure LPUART11 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART11_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart11, &rootCfg);

    /* Configure LPUART12 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPUART12_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart12, &rootCfg);

    /* Configure LPI2C1 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPI2C1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c1, &rootCfg);

    /* Configure LPI2C2 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPI2C2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c2, &rootCfg);

    /* Configure LPI2C3 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPI2C3_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c3, &rootCfg);

    /* Configure LPI2C4 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPI2C4_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c4, &rootCfg);

    /* Configure LPI2C5 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPI2C5_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c5, &rootCfg);

    /* Configure LPI2C6 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPI2C6_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c6, &rootCfg);

    /* Configure LPSPI1 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPSPI1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpspi1, &rootCfg);

    /* Configure LPSPI2 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPSPI2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpspi2, &rootCfg);

    /* Configure LPSPI3 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPSPI3_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpspi3, &rootCfg);

    /* Configure LPSPI4 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPSPI4_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpspi4, &rootCfg);

    /* Configure LPSPI5 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPSPI5_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpspi5, &rootCfg);

    /* Configure LPSPI6 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_LPSPI6_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpspi6, &rootCfg);

    /* Configure ENET2 using SYS_PLL1_DIV2_CLK */
    rootCfg.mux = kCLOCK_ENET2_ClockRoot_MuxSysPll1Div2;
    rootCfg.div = 4;
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);

    /* Configure ENET_QOS using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_ENET_QOS_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Enet_Qos, &rootCfg);

    /* Configure ENET_25M using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_ENET_25M_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Enet_25m, &rootCfg);

    /* Configure ENET_TIMER1 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_ENET_TIMER1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Enet_Timer1, &rootCfg);

    /* Configure ENET_TIMER2 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_ENET_TIMER2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Enet_Timer2, &rootCfg);

    /* Configure ENET_TIMER3 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_ENET_TIMER3_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Enet_Timer3, &rootCfg);

    /* Configure USDHC1 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_USDHC1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);

    /* Configure MQS using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_MQS_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Mqs, &rootCfg);

    /* Configure MIC using PLL_AUDIO_CLK */
    rootCfg.mux = kCLOCK_MIC_ClockRoot_MuxAudioPllOut;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Mic, &rootCfg);

    /* Configure SAI1 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_SAI1_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Sai1, &rootCfg);

    /* Configure SAI2 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_SAI2_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Sai2, &rootCfg);

    /* Configure SAI3 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_SAI3_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Sai3, &rootCfg);

    /* Configure SAI4 using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_SAI4_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Sai4, &rootCfg);

    /* Configure CSI using OSC_RC_48M_DIV2 */
    rootCfg.mux = kCLOCK_CSI_ClockRoot_MuxOscRc48MDiv2;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Csi, &rootCfg);

    /* Set SAI1 MCLK1 clock source. */
    IOMUXC_SetSaiMClkClockSource(IOMUXC_GPR, kIOMUXC_GPR_SAI1MClk1Sel, 0);
    /* Set SAI1 MCLK2 clock source. */
    IOMUXC_SetSaiMClkClockSource(IOMUXC_GPR, kIOMUXC_GPR_SAI1MClk2Sel, 3);
    /* Set SAI1 MCLK3 clock source. */
    IOMUXC_SetSaiMClkClockSource(IOMUXC_GPR, kIOMUXC_GPR_SAI1MClk3Sel, 0);
    /* Set SAI2 MCLK3 clock source. */
    IOMUXC_SetSaiMClkClockSource(IOMUXC_GPR, kIOMUXC_GPR_SAI2MClk3Sel, 0);
    /* Set SAI3 MCLK3 clock source. */
    IOMUXC_SetSaiMClkClockSource(IOMUXC_GPR, kIOMUXC_GPR_SAI3MClk3Sel, 0);

    /* Set MQS configuration. */
    IOMUXC_MQSConfig(IOMUXC_GPR,kIOMUXC_MqsPwmOverSampleRate32, 0);
    /* Set ENET Ref clock source. */
    IOMUXC_GPR->GPR4 &= ~IOMUXC_GPR_GPR4_ENET_REF_CLK_DIR_MASK;
    /* Set ENET_1G Tx clock source. */
    IOMUXC_GPR->GPR5 = ((IOMUXC_GPR->GPR5 & ~IOMUXC_GPR_GPR5_ENET1G_TX_CLK_SEL_MASK) | IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK);
    /* Set ENET_1G Ref clock source. */
    IOMUXC_GPR->GPR5 &= ~IOMUXC_GPR_GPR5_ENET1G_REF_CLK_DIR_MASK;
    /* Set ENET_QOS Tx clock source. */
    IOMUXC_GPR->GPR6 &= ~IOMUXC_GPR_GPR6_ENET_QOS_RGMII_EN_MASK;
    /* Set ENET_QOS Ref clock source. */
    IOMUXC_GPR->GPR6 &= ~IOMUXC_GPR_GPR6_ENET_QOS_REF_CLK_DIR_MASK;
    /* Set GPT1 High frequency reference clock source. */
    IOMUXC_GPR->GPR22 &= ~IOMUXC_GPR_GPR22_REF_1M_CLK_GPT1_MASK;
    /* Set GPT2 High frequency reference clock source. */
    IOMUXC_GPR->GPR23 &= ~IOMUXC_GPR_GPR23_REF_1M_CLK_GPT2_MASK;
    /* Set GPT3 High frequency reference clock source. */
    IOMUXC_GPR->GPR24 &= ~IOMUXC_GPR_GPR24_REF_1M_CLK_GPT3_MASK;
    /* Set GPT4 High frequency reference clock source. */
    IOMUXC_GPR->GPR25 &= ~IOMUXC_GPR_GPR25_REF_1M_CLK_GPT4_MASK;
    /* Set GPT5 High frequency reference clock source. */
    IOMUXC_GPR->GPR26 &= ~IOMUXC_GPR_GPR26_REF_1M_CLK_GPT5_MASK;
    /* Set GPT6 High frequency reference clock source. */
    IOMUXC_GPR->GPR27 &= ~IOMUXC_GPR_GPR27_REF_1M_CLK_GPT6_MASK;

#if __CORTEX_M == 7
    SystemCoreClock = CLOCK_GetRootClockFreq(kCLOCK_Root_M7);
#else
    SystemCoreClock = CLOCK_GetRootClockFreq(kCLOCK_Root_M4);
#endif
}
