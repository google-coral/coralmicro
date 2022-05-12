#ifndef _LIBS_BASE_MAIN_FREERTOS_M7_H_
#define _LIBS_BASE_MAIN_FREERTOS_M7_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#if defined(__cplusplus)
extern "C" {
#endif

int real_main(int argc, char** argv, bool init_console_tx,
              bool init_console_rx);
lpi2c_rtos_handle_t* I2C5Handle();

#if defined(__cplusplus)
}
#endif

#endif  // _LIBS_BASE_MAIN_FREERTOS_M7_H_
