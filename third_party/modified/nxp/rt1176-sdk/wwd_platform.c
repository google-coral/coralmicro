/*
 * Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 * Defines IMXRT1050-specific WWD platform functions
 */
#include <stdio.h>
#include "wwd_constants.h"
#include "wwd_platform_common.h" //ToDo : check whether to use the enums
#include "platform/wwd_platform_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_debug.h"

//IMXRT platform headers
#include "fsl_gpio.h"
#include "fsl_sdio.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"

/******************************************************
 *                      Macros
 ******************************************************/
#if defined(BOARD_INITPINS_WL_REG_ON_GPIO) && defined(BOARD_INITPINS_WL_REG_ON_PIN)
    /*
     * Support for LPC GPIO driver
     */
    #ifdef _LPC_GPIO_H_
        #define SET_WL_REG_ON(value) GPIO_PinWrite( BOARD_INITPINS_WL_REG_ON_GPIO, BOARD_INITPINS_WL_REG_ON_PORT, BOARD_INITPINS_WL_REG_ON_PIN, value );
    #else
        #define SET_WL_REG_ON(value) GPIO_PinWrite( BOARD_INITPINS_WL_REG_ON_GPIO, BOARD_INITPINS_WL_REG_ON_PIN, value );
    #endif
#else
    #warning "Please define BOARD_INITPINS_WL_REG_ON_GPIO and BOARD_INITPINS_WL_REG_ON_PIN in pin_mux.h"
    #define SET_WL_REG_ON(value)
#endif

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern sdio_card_t g_sdio;
/******************************************************
 *               Function Definitions
 ******************************************************/

static void host_platform_wifi_pwr_pin_init(void)
{
    /* nothing to do */
}

static void host_platform_wl_reg_on_set_low(void)
{
    SET_WL_REG_ON(0U);
}

static void host_platform_wl_reg_on_set_high(void)
{
    /*
     * 1DX M.2 Module Datasheet and 1LV M.2 Module Datasheet (EA2-DS-1901):
     * Signals WL_REG_ON or BT_REG_ON must be held low for at least 700 microseconds
     * after supply voltage has reached specification level before pulled high.
     * 2 clock cycles of the 32.678kHz clock must also have passed before any of the signals is pulled high.
     * These clock cycles will typically occur during the 700 microseconds
     * but if the clock signal has a long delay during power-up, the 700 microsecond period can be extended.
     */
    host_rtos_delay_milliseconds( (uint32_t) 1 );

    SET_WL_REG_ON(1U);
}

wwd_result_t host_platform_init( void )
{
    wwd_result_t err = WWD_SUCCESS;

    // Ensure WL_REG_ON is low
    host_platform_wl_reg_on_set_low();

    // Initialize GPIO pins for wifi power control
    host_platform_wifi_pwr_pin_init();

    // Set WL_REG_ON high
    host_platform_wl_reg_on_set_high();

    return err;
}

wwd_result_t host_platform_deinit(void)
{
    host_platform_wl_reg_on_set_low();

    return WWD_SUCCESS;
}

void host_platform_reset_wifi( wiced_bool_t reset_asserted )
{
    if ( reset_asserted == WICED_TRUE )
    {
        SDIO_SetCardPower(&g_sdio, false);
        host_platform_wl_reg_on_set_low();
        host_rtos_delay_milliseconds( (uint32_t) 100 );
    }
    else
    {
        SDIO_SetCardPower(&g_sdio, true);
        host_platform_wl_reg_on_set_high();
        host_rtos_delay_milliseconds( (uint32_t) 100 );
    }
}

void host_platform_power_wifi( wiced_bool_t power_enabled )
{
    UNUSED_PARAMETER( power_enabled );
}

wwd_result_t host_platform_init_wlan_powersave_clock( void )
{
	//ToDo : cleanup
#if defined ( WICED_USE_WIFI_32K_CLOCK_MCO )
    platform_gpio_set_alternate_function( wifi_control_pins[WWD_PIN_32K_CLK].port, wifi_control_pins[WWD_PIN_32K_CLK].pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_MCO );

    /* enable LSE output on MCO1 */
    RCC_MCO1Config( RCC_MCO1Source_LSE, RCC_MCO1Div_1 );
#endif

    return WWD_SUCCESS;
}

// wiced_bool_t host_platform_is_in_interrupt_context( void )
// {
//     return __get_IPSR();
// }
