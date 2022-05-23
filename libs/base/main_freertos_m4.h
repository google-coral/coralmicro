#ifndef LIBS_BASE_MAIN_FREERTOS_M4_H_
#define LIBS_BASE_MAIN_FREERTOS_M4_H_

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#if defined(__cplusplus)
extern "C" {
#endif

lpi2c_rtos_handle_t* I2C5Handle();

#if defined(__cplusplus)
}
#endif

#endif  // LIBS_BASE_MAIN_FREERTOS_M4_H_
