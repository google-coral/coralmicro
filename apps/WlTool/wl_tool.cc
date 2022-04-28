#include <cstdio>
#include <cstring>

#include "libs/rpc/rpc_http_server.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/wwd_wiced.h"

SemaphoreHandle_t lpuart7_rx_mtx;
SemaphoreHandle_t lpuart7_tx_mtx;

/* See third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/test/wl_tool/43455C0/wl/exe/wlu_remote.h */
constexpr int kSerialSuccess = 1;
constexpr int kSerialFail = -1;
constexpr int kSerialNoPacket = -2;
constexpr int kSerialPortErr = -3;

constexpr int kLpuart7RingBufferSize = 2048;
static SERIAL_MANAGER_HANDLE_DEFINE(lpuart7_handle);
static SERIAL_MANAGER_READ_HANDLE_DEFINE(lpuart7_read_handle);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(lpuart7_write_handle);
static uint8_t lpuart7_ring_buffer[kLpuart7RingBufferSize];

extern "C" wiced_result_t wiced_wlan_connectivity_init(void);
extern "C" int remote_server_exec(int argc, char **argv, void *ifr);

void lpuart7_Callback(void *callbackParam,
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

extern "C" int rwl_write_serial_port(void *hndle, char *write_buf,
                                     unsigned long size,
                                     unsigned long *numwritten) {
  if (!write_buf || size == 0 || !numwritten) {
    return kSerialPortErr;
  }

  serial_manager_status_t status = SerialManager_WriteNonBlocking(
      lpuart7_write_handle, reinterpret_cast<uint8_t *>(write_buf), size);
  if (status == kStatus_SerialManager_Success) {
    if (pdTRUE == xSemaphoreTake(lpuart7_tx_mtx, portMAX_DELAY)) {
      *numwritten = size;
      return kSerialSuccess;
    } else {
      return kSerialPortErr;
    }
  } else {
    return kSerialPortErr;
  }
}

extern "C" int rwl_read_serial_port(void *hndle, char *read_buf,
                                    unsigned int data_size,
                                    unsigned int *numread) {
  if (data_size == 0) {
    return kSerialSuccess;
  }
  if (!read_buf || !numread) {
    return kSerialPortErr;
  }

  serial_manager_status_t status = SerialManager_ReadNonBlocking(
      lpuart7_read_handle, reinterpret_cast<uint8_t *>(read_buf), data_size);
  if (status == kStatus_SerialManager_Success) {
    if (pdTRUE == xSemaphoreTake(lpuart7_rx_mtx, portMAX_DELAY)) {
      *numread = data_size;
      return kSerialSuccess;
    } else {
      return kSerialPortErr;
    }
  } else {
    return kSerialPortErr;
  }
}

extern "C" void app_main(void *param) {
  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export(valiant::testlib::kMethodWifiSetAntenna,
                 valiant::testlib::WifiSetAntenna);
  valiant::UseHttpServer(new valiant::JsonRpcHttpServer);

  wwd_result_t err;
  err = (wwd_result_t)wiced_wlan_connectivity_init();
  if (err != WWD_SUCCESS) {
    printf("wlan init failed\r\n");
    vTaskSuspend(NULL);
  }

  // Configure UART root clock to target ~24MHz (PLL2 = 533MHz)
  clock_root_config_t rootCfg = {0};
  rootCfg.mux = kCLOCK_LPUART7_ClockRoot_MuxSysPll2Out;
  rootCfg.div = 22;
  CLOCK_SetRootClock(kCLOCK_Root_Lpuart7, &rootCfg);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_00_LPUART7_TXD, 0U);
  IOMUXC_SetPinMux(IOMUXC_GPIO_AD_01_LPUART7_RXD, 0U);

  serial_manager_config_t config;
  serial_port_uart_config_t uartConfig;
  config.type = kSerialPort_Uart;
  config.ringBuffer = lpuart7_ring_buffer;
  config.ringBufferSize = kLpuart7RingBufferSize;
  uartConfig.instance = 7;
  uartConfig.clockRate = CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart7);
  uartConfig.baudRate = 115200;
  uartConfig.parityMode = kSerialManager_UartParityDisabled;
  uartConfig.stopBitCount = kSerialManager_UartOneStopBit;
  uartConfig.enableRx = 1;
  uartConfig.enableTx = 1;
  uartConfig.enableRxRTS = 0;
  uartConfig.enableTxCTS = 0;
  config.portConfig = &uartConfig;
  NVIC_SetPriority(LPUART7_IRQn, 5);
  serial_manager_status_t status = SerialManager_Init(lpuart7_handle, &config);

  if (status != kStatus_SerialManager_Success) {
    printf("UART init failed\r\n");
    vTaskSuspend(NULL);
  }

  status = SerialManager_OpenReadHandle(lpuart7_handle, lpuart7_read_handle);
  if (status != kStatus_SerialManager_Success) {
    printf("UART read handle failed\r\n");
    vTaskSuspend(NULL);
  }

  lpuart7_rx_mtx = xSemaphoreCreateBinary();
  status = SerialManager_InstallRxCallback(lpuart7_read_handle,
                                           lpuart7_Callback, lpuart7_rx_mtx);
  if (status != kStatus_SerialManager_Success) {
    printf("Failed to install UART read callback\r\n");
    vTaskSuspend(NULL);
  }

  status = SerialManager_OpenWriteHandle(lpuart7_handle, lpuart7_write_handle);
  if (status != kStatus_SerialManager_Success) {
    printf("UART write handle failed\r\n");
    vTaskSuspend(NULL);
  }

  lpuart7_tx_mtx = xSemaphoreCreateBinary();
  status = SerialManager_InstallTxCallback(lpuart7_write_handle,
                                           lpuart7_Callback, lpuart7_tx_mtx);
  if (status != kStatus_SerialManager_Success) {
    printf("Failed to install UART write callback\r\n");
    vTaskSuspend(NULL);
  }

  remote_server_exec(0, nullptr, nullptr);

  SerialManager_CloseReadHandle(lpuart7_read_handle);
  SerialManager_CloseWriteHandle(lpuart7_write_handle);

  vTaskSuspend(NULL);
}
