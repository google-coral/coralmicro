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

#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

SemaphoreHandle_t lpuart2_rx_mtx;
SemaphoreHandle_t lpuart2_tx_mtx;
constexpr int kLpuart2RingBufferSize = 256;
static SERIAL_MANAGER_HANDLE_DEFINE(lpuart2_handle);
static SERIAL_MANAGER_READ_HANDLE_DEFINE(lpuart2_read_handle);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(lpuart2_write_handle);
static uint8_t lpuart2_ring_buffer[kLpuart2RingBufferSize];

void ble_pwr_on(void) {
  vTaskDelay(pdMS_TO_TICKS(500));
  coralmicro::GpioSet(coralmicro::Gpio::kBtRegOn, true);
  vTaskDelay(pdMS_TO_TICKS(500));
}

void ble_pwr_off(void) {
  vTaskDelay(pdMS_TO_TICKS(500));
  coralmicro::GpioSet(coralmicro::Gpio::kBtRegOn, false);
  vTaskDelay(pdMS_TO_TICKS(500));
}

extern "C" void *bte_port_malloc(uint32_t size) { return malloc(size); }

extern "C" void bte_port_free(void *mem) { free(mem); }

void lpuart2_Callback(void *callbackParam,
                      serial_manager_callback_message_t *message,
                      serial_manager_status_t status) {
  SemaphoreHandle_t semaphore =
      reinterpret_cast<SemaphoreHandle_t>(callbackParam);
  if (status == kStatus_SerialManager_Success) {
    // If IPSR is non-zero, we are in an interrupt.
    // Use the correct FreeRTOS APIs for that context.
    if (__get_IPSR() != 0) {
      BaseType_t reschedule;
      xSemaphoreGiveFromISR(semaphore, &reschedule);
      portYIELD_FROM_ISR(reschedule);
    } else {
      xSemaphoreGive(semaphore);
    }
  }
}

extern "C" int imxrt_bt_uart_init(void) {
  lpuart2_rx_mtx = xSemaphoreCreateBinary();
  lpuart2_tx_mtx = xSemaphoreCreateBinary();

  serial_manager_config_t config;
  serial_port_uart_config_t uartConfig;
  config.type = kSerialPort_Uart;
  config.ringBuffer = lpuart2_ring_buffer;
  config.ringBufferSize = kLpuart2RingBufferSize;
  uartConfig.instance = 2;
  uartConfig.clockRate = CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart2);
  uartConfig.baudRate = 115200;
  uartConfig.parityMode = kSerialManager_UartParityDisabled;
  uartConfig.stopBitCount = kSerialManager_UartOneStopBit;
  uartConfig.enableRx = 1;
  uartConfig.enableTx = 1;
  uartConfig.enableRxRTS = 1;
  uartConfig.enableTxCTS = 1;
  config.portConfig = &uartConfig;
  NVIC_SetPriority(LPUART2_IRQn, 5);

  serial_manager_status_t status = SerialManager_Init(lpuart2_handle, &config);
  if (status != kStatus_SerialManager_Success) {
    return kStatus_Fail;
  }

  status = SerialManager_OpenReadHandle(lpuart2_handle, lpuart2_read_handle);
  if (status != kStatus_SerialManager_Success) {
    return kStatus_Fail;
  }

  status = SerialManager_OpenWriteHandle(lpuart2_handle, lpuart2_write_handle);
  if (status != kStatus_SerialManager_Success) {
    return kStatus_Fail;
  }

  SerialManager_InstallTxCallback(lpuart2_write_handle, lpuart2_Callback,
                                  lpuart2_tx_mtx);
  SerialManager_InstallRxCallback(lpuart2_read_handle, lpuart2_Callback,
                                  lpuart2_rx_mtx);

  ble_pwr_on();

  return kStatus_Success;
}

extern "C" int imxrt_bt_uart_deinit(void) {
  SerialManager_CloseReadHandle(lpuart2_read_handle);
  SerialManager_CloseWriteHandle(lpuart2_write_handle);
  SerialManager_Deinit(lpuart2_handle);

  vSemaphoreDelete(lpuart2_tx_mtx);
  vSemaphoreDelete(lpuart2_rx_mtx);

  ble_pwr_off();

  return kStatus_Success;
}

extern "C" int imxrt_bt_uart_set_baudrate(uint32_t baudrate) {
  return LPUART_SetBaudRate(LPUART2, baudrate,
                            CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart2));
}

extern "C" int imxrt_bt_uart_write(uint8_t *buf, int len) {
  serial_manager_status_t status =
      SerialManager_WriteNonBlocking(lpuart2_write_handle, buf, len);
  if (status == kStatus_SerialManager_Success) {
    if (pdTRUE == xSemaphoreTake(lpuart2_tx_mtx, portMAX_DELAY)) {
      return kStatus_Success;
    }
    return kStatus_Fail;
  }
  return kStatus_Fail;
}

extern "C" int imxrt_bt_uart_read(uint8_t *buf, int len, int *read_len) {
  serial_manager_status_t status =
      SerialManager_ReadNonBlocking(lpuart2_read_handle, buf, len);
  if (status == kStatus_SerialManager_Success) {
    if (pdTRUE == xSemaphoreTake(lpuart2_rx_mtx, portMAX_DELAY)) {
      if (read_len) {
        *read_len = len;
      }
      return kStatus_Success;
    }
    return kStatus_Fail;
  }
  return kStatus_Fail;
}
