
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
 * Defines WWD SDIO functions for STM32F4xx MCU
 */

// WWD Headers
#include <string.h> /* For memcpy */
#include "wwd_platform_common.h"
// #include "wwd_bus_protocol.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
// #include "platform/wwd_sdio_interface.h"
// #include "platform/wwd_bus_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "network/wwd_network_constants.h"
#include "platform_peripheral.h"
#include "chip_constants.h"
// #include "wwd.h"

// IMXRT Platform headers
#include "fsl_sdio.h"
#include "fsl_gpio.h"
//#include "fsl_iomuxc.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_sdmmc_host.h"
#include "fsl_sdmmc_spec.h"
#include "fsl_usdhc.h"
#include "sdmmc_config.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST  (1)
#define SDIO_ENUMERATION_TIMEOUT_MS            (500)

#define SDIO_TransferDir_ToCard             ((uint32_t)0x00000000)
#define SDIO_TransferDir_ToSDIO             ((uint32_t)0x00000002)
#define IS_SDIO_TRANSFER_DIR(DIR)           (((DIR) == SDIO_TransferDir_ToCard) || \
                                             ((DIR) == SDIO_TransferDir_ToSDIO))

//#define USE_FSL_SDIO 1

/*  Wi-Fi GPIO0 pin is used for out-of-band interrupt */
#define WICED_WIFI_OOB_IRQ_GPIO_PIN  ( 0 )
/******************************************************
 *             Constants
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

typedef struct
{
    /*@shared@*/ /*@null@*/ uint8_t* data;
    uint16_t length;
} sdio_dma_segment_t;

typedef enum
{
    SDIO_CMD_0  =  0,
    SDIO_CMD_3  =  3,
    SDIO_CMD_5  =  5,
    SDIO_CMD_7  =  7,
    SDIO_CMD_52 = 52,
    SDIO_CMD_53 = 53,
    __MAX_VAL   = 64
} wwd_sdio_command_t;
typedef enum
{
    SDIO_BLOCK_MODE = ( 0 << 2 ), /* These are STM32 implementation specific */
    SDIO_BYTE_MODE  = ( 1 << 2 )  /* These are STM32 implementation specific */
} sdio_transfer_mode_t;
typedef enum
{
    SDIO_1B_BLOCK    =  1,
    SDIO_2B_BLOCK    =  2,
    SDIO_4B_BLOCK    =  4,
    SDIO_8B_BLOCK    =  8,
    SDIO_16B_BLOCK   =  16,
    SDIO_32B_BLOCK   =  32,
    SDIO_64B_BLOCK   =  64,
    SDIO_128B_BLOCK  =  128,
    SDIO_256B_BLOCK  =  256,
    SDIO_512B_BLOCK  =  512,
    SDIO_1024B_BLOCK = 1024,
    SDIO_2048B_BLOCK = 2048
} sdio_block_size_t;
typedef enum
{
    RESPONSE_NEEDED,
    NO_RESPONSE
} sdio_response_needed_t;
typedef enum
{
    WWD_NETWORK_TX,
    WWD_NETWORK_RX
} wwd_buffer_dir_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static sdio_block_size_t find_optimal_block_size( uint32_t data_size );
/*!
* @brief call back function for SD card detect.
*
* @param isInserted  true,  indicate the card is insert.
*                    false, indicate the card is remove.
* @param userData
*/
static void SDIOCARD_DetectCallBack(bool isInserted, void *userData);

/******************************************************
 *               Function Declarations
 ******************************************************/
void  host_platform_sdio_irq_callback(void *userData);
wwd_result_t host_platform_sdio_transfer_internal( wwd_bus_transfer_direction_t direction,
		wwd_sdio_command_t command,
		sdio_transfer_mode_t mode,
		sdio_block_size_t block_size,
		uint32_t argument,
		/*@null@*/ uint32_t* pdata,
		uint16_t data_size,
		sdio_response_needed_t response_expected,
		usdhc_card_response_type_t response_type,
		uint32_t response_error_flags,
		/*@out@*/ /*@null@*/ uint32_t* response );

/******************************************************
 *             Variables
 ******************************************************/
/*!
 * @brief Card descriptor.
 * */
sdio_card_t g_sdio;
sdio_bus_width_t g_buswidth = kSDIO_DataBus1Bit;
/*!
 * @brief SD card detect flag
 * */
static bool s_wificardInserted = false;
/******************************************************
 *             Function definitions
 ******************************************************/
static void SDIOCARD_DetectCallBack(bool isInserted, void *userData)
{
	s_wificardInserted = isInserted;
}

#ifndef WICED_DISABLE_MCU_POWERSAVE
wwd_result_t host_enable_oob_interrupt( void )
{
	//ToDo
    /* Set GPIO_B[1:0] to input. One of them will be re-purposed as OOB interrupt */
    // platform_gpio_init( &wifi_sdio_pins[WWD_PIN_SDIO_OOB_IRQ], INPUT_HIGH_IMPEDANCE );
    //platform_gpio_irq_enable( &wifi_sdio_pins[WWD_PIN_SDIO_OOB_IRQ], IRQ_TRIGGER_RISING_EDGE, sdio_oob_irq_handler, 0 );

    return WWD_SUCCESS;
}

wwd_result_t host_platform_unmask_sdio_interrupt(void)
{
	//Clear the CardInterrupt status bit
	USDHC_ClearInterruptStatusFlags(g_sdio.host->hostController.base,kUSDHC_CardInterruptFlag);

	//Enable the CardInterrupt Signal
	USDHC_EnableInterruptSignal(g_sdio.host->hostController.base,kUSDHC_CardInterruptFlag);

    return WWD_SUCCESS;
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
    return WICED_WIFI_OOB_IRQ_GPIO_PIN;
}

#endif /* ifndef  WICED_DISABLE_MCU_POWERSAVE */

wwd_result_t host_platform_bus_init( void )
{
    status_t err = kStatus_Success;

    WPRINT_WWD_DEBUG(("Source Clk: %d\n",g_sdio.host->hostController.sourceClock_Hz ));

    BOARD_SDIO_Config(&g_sdio, SDIOCARD_DetectCallBack, 5U, host_platform_sdio_irq_callback);

    /* SDIO host init function */
    err = SDIO_HostInit(&g_sdio);
    if (err != kStatus_Success)
    {
		WPRINT_WWD_INFO(("\n SDIO host init fail \n"));
		return (wwd_result_t)err;
    }

    /* card detect */
    if (SDIO_PollingCardInsert(&g_sdio, kSD_Inserted) != kStatus_Success)
    {
        WPRINT_WWD_INFO(("\n SDIO card detect failed \n"));
        return (wwd_result_t)err;
    }
    /* power off card */
    SDIO_SetCardPower(&g_sdio, false);
    /* power on card */
    SDIO_SetCardPower(&g_sdio, true);

    /* initialize bus clock */
    g_sdio.busClock_Hz = SDMMCHOST_SetCardClock(g_sdio.host, SDMMC_CLOCK_400KHZ);

    /* get host capability */
    WPRINT_WWD_DEBUG(("Bus Clock Max %d block length %d\n",g_sdio.busClock_Hz,SDMMCHOST_SUPPORT_MAX_BLOCK_LENGTH));

    return WWD_SUCCESS;
}

wwd_result_t host_platform_sdio_enumerate( void )
{
    wwd_result_t result=WWD_SUCCESS;

#ifdef USE_FSL_SDIO // IMXRT SDIO driver performs the same operation in SDIO_CardInit( )

    status_t err = kStatus_Success;
    if(s_wificardInserted)
    {
    	err = SDIO_CardInit(&g_sdio);
    	if (err != kStatus_Success)
		{
			WPRINT_WWD_INFO(("\n SDIO/Wifi Card init fail \n"));
			return (wwd_result_t)err;
		}
    }
#else
    uint32_t       loop_count;
    uint32_t       data = 0;
    loop_count = 0;
    do
    {
        /* Send CMD0 to set it to idle state */
    	result = host_platform_sdio_transfer_internal( BUS_WRITE, SDIO_CMD_0, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, kCARD_ResponseTypeNone,0, &data );
    	WPRINT_WWD_DEBUG(("\n Sent CMD0 \n"));

        /* CMD5. */
    	result = host_platform_sdio_transfer_internal( BUS_READ, SDIO_CMD_5, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, kCARD_ResponseTypeR4,0, &data );
    	if(result == WWD_SUCCESS)
    	{
    		WPRINT_WWD_DEBUG(("\n Sent CMD5 \n"));
    		g_sdio.memPresentFlag = data & kSDIO_OcrMemPresent;
    		g_sdio.ioTotalNumber =  data & kSDIO_OcrIONumber;
    	}

        /* Send CMD3 to get RCA. */
        result = host_platform_sdio_transfer_internal( BUS_READ, SDIO_CMD_3, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, RESPONSE_NEEDED, kCARD_ResponseTypeR6,0, &data );
    	if(result == WWD_SUCCESS)
    	{
    		WPRINT_WWD_DEBUG(("\n Sent CMD3 \n"));
    		g_sdio.relativeAddress = data >> 16 ;
    	}

        loop_count++;
        if ( loop_count >= (uint32_t) SDIO_ENUMERATION_TIMEOUT_MS )
        {
            return WWD_TIMEOUT;
        }
    } while ( ( result != WWD_SUCCESS ) && ( host_rtos_delay_milliseconds( (uint32_t) 1 ), ( 1 == 1 ) ) );
    /* If you're stuck here, check the platform matches your hardware */
    /* Send CMD7 with the returned RCA to select the card */

    result = host_platform_sdio_transfer_internal( BUS_WRITE, SDIO_CMD_7,
    									SDIO_BYTE_MODE,
										SDIO_1B_BLOCK,
										data, 0, 0,
										RESPONSE_NEEDED,
										kCARD_ResponseTypeR1, 0, &data );
    if(result == WWD_SUCCESS)
    {
    	WPRINT_WWD_DEBUG(("\n Sent CMD7 \n"));
    }


#endif
    return result;
}

wwd_result_t host_platform_bus_deinit( void )
{
	return WWD_SUCCESS;
}

wwd_result_t host_platform_sdio_transfer_internal( wwd_bus_transfer_direction_t direction,
		wwd_sdio_command_t command,
		sdio_transfer_mode_t mode,
		sdio_block_size_t block_size,
		uint32_t argument,
		/*@null@*/ uint32_t* pdata,
		uint16_t data_size,
		sdio_response_needed_t response_expected,
		usdhc_card_response_type_t response_type,
		uint32_t response_error_flags,
		/*@out@*/ /*@null@*/ uint32_t* response )
{
	wwd_result_t result = WWD_SUCCESS;

	sdmmchost_transfer_t content = {0U};
	sdmmchost_cmd_t xcommand = {0U};
	sdmmchost_data_t xdata = {0};

	xcommand.index = command;
	xcommand.argument = argument;
	xcommand.responseType = response_type/*kCARD_ResponseTypeR5*/;
	xcommand.responseErrorFlags =response_error_flags/*0*/;
	// DbgConsole_Printf("xcommand: %d %d %d %d\r\n", command, argument, response_type, response_error_flags);
	// DbgConsole_Printf("pdata: %p\r\n", pdata);

	// Set the transmit or receive data buffer
	if(direction == BUS_WRITE)
	{
		xdata.txData = pdata;
	}
	else if(direction == BUS_READ)
	{
		xdata.rxData = pdata;
	}

	//Set Block/byte mode settings
	if(mode == SDIO_BLOCK_MODE)
	{
		xdata.blockCount = ( argument & (uint32_t) (0x1FF ) );//( data_size / (uint16_t)SDIO_64B_BLOCK );
		xdata.blockSize = block_size ;
		// DbgConsole_Printf("block mode %d %d\r\n", xdata.blockCount, xdata.blockSize);
	}
	else if(mode == SDIO_BYTE_MODE)
	{
		// DbgConsole_Printf("byte mode\r\n");
		xdata.blockCount = 1U;
		block_size = find_optimal_block_size( data_size );
		if ( block_size < SDIO_512B_BLOCK )
		{
			argument = ( argument & (uint32_t) ( ~0x1FF ) ) | block_size;
		}
		else
		{
			argument = ( argument & (uint32_t) ( ~0x1FF ) );
		}
		xdata.blockSize = (size_t)data_size;
	}

	content.command = &xcommand;
	content.data = (pdata == 0 ) ? NULL : &xdata;

	if (kStatus_Success != SDMMCHOST_TransferFunction(g_sdio.host, &content))
	{
		// DbgConsole_Printf("%s %d\r\n", __func__, __LINE__);
	     return (wwd_result_t)kStatus_SDMMC_TransferFailed;
	}

	/* read data from response */
	if (response != NULL)
	{
		*response = xcommand.response[0U];
	}
    return result;
}

wwd_result_t host_platform_enable_high_speed_sdio( void )
{
	status_t result = kStatus_Success;

	result = SDIO_GetCardCapability(&g_sdio, kSDIO_FunctionNum0 );
	if(result != kStatus_Success)
	{
		WPRINT_WWD_INFO(("\n Get card capability fail \n"));
	}

	result = SDIO_EnableAsyncInterrupt(&g_sdio, true );
	if(result != kStatus_Success)
	{
		if(result == kStatus_SDMMC_NotSupportYet)
		{
			WPRINT_WWD_INFO(("\n AsyncInterrupt is not supported \n"));
		}
		else
		{
			WPRINT_WWD_INFO(("\n EnableAsyncInterrupt fail \n"));
		}
	}

        /* switch to clock frequency supported by the card (25 or 50 MHz) */
        result = SDIO_SwitchToHighSpeed(&g_sdio);
        if(result != kStatus_Success)
        {
                WPRINT_WWD_INFO(("\n Switch to high speed fail \n"));
        }
        else
        {
                WPRINT_WWD_DEBUG(("\n Switched to Highspeed with freq %u \n",g_sdio.busClock_Hz));
        }

	if ( g_buswidth == kSDIO_DataBus4Bit)
	{
		// DbgConsole_Printf("Set 4 bit\r\n");
		SDMMCHOST_SetCardBusWidth(g_sdio.host, kSDMMC_BusWdith4Bit);
	}
	else
	{
		// DbgConsole_Printf("Set 1 bit\r\n");
		SDMMCHOST_SetCardBusWidth(g_sdio.host, kSDMMC_BusWdith1Bit);
	}
    return (wwd_result_t)result;
}

static sdio_block_size_t find_optimal_block_size( uint32_t data_size )
{
    if ( data_size > (uint32_t) 256 )
        return SDIO_512B_BLOCK;
    if ( data_size > (uint32_t) 128 )
        return SDIO_256B_BLOCK;
    if ( data_size > (uint32_t) 64 )
        return SDIO_128B_BLOCK;
    if ( data_size > (uint32_t) 32 )
        return SDIO_64B_BLOCK;
    if ( data_size > (uint32_t) 16 )
        return SDIO_32B_BLOCK;
    if ( data_size > (uint32_t) 8 )
        return SDIO_16B_BLOCK;
    if ( data_size > (uint32_t) 4 )
        return SDIO_8B_BLOCK;
    if ( data_size > (uint32_t) 2 )
        return SDIO_4B_BLOCK;

    return SDIO_4B_BLOCK;
}

wwd_result_t host_platform_bus_enable_interrupt( void )
{
	USDHC_EnableInterruptStatus(g_sdio.host->hostController.base, kUSDHC_CardInterruptFlag);
    return  WWD_SUCCESS;
}

wwd_result_t host_platform_bus_disable_interrupt( void )
{
	USDHC_DisableInterruptStatus(g_sdio.host->hostController.base, kUSDHC_CardInterruptFlag);
    return  WWD_SUCCESS;
}

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{
    UNUSED_PARAMETER( direction );
}

/******************************************************
 *             IRQ Handler Definitions
 ******************************************************/
void  host_platform_sdio_irq_callback(void *userData)
{
    wwd_thread_notify_irq();
    USDHC_DisableInterruptSignal(g_sdio.host->hostController.base, USDHC_INT_SIGNAL_EN_CINTIEN_MASK);
    USDHC_ClearInterruptStatusFlags(g_sdio.host->hostController.base, USDHC_INT_STATUS_CINT_MASK);
}
