#ifndef _APPS_HELLOWORLD_BOARD_H_
#define _APPS_HELLOWORLD_BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"

#define BOARD_NAME "MIMXRT1170-EVK"
#define DEBUG_CONSOLE_UART_INDEX 1

#define BOARD_DEBUG_UART_TYPE kSerialPort_Uart
#define BOARD_DEBUG_UART_CLK_FREQ 24000000
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART1
#define BOARD_DEBUG_UART_INSTANCE 1U
#define BOARD_UART_IRQ LPUART1_IRQn
#define BOARD_UART_IRQ_HANDLER LPUART1_IRQHandler
#define BOARD_DEBUG_UART_BAUDRATE (115200U)

#if defined(__cplusplus)
extern "C" {
#endif  // defined(__cplusplus)

uint32_t BOARD_DebugConsoleSrcFreq(void);
void BOARD_InitDebugConsole(void);
void BOARD_ConfigMPU(void);

#if defined(__cplusplus)
}
#endif  // defined(__cplusplus)

#endif  // _APPS_HELLOWORLD_BOARD_H_
