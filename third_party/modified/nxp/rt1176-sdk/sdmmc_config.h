#ifndef _LIBS_NXP_RT1176_SDK_SDMMC_CONFIG_H_
#define _LIBS_NXP_RT1176_SDK_SDMMC_CONFIG_H_

#include "third_party/nxp/rt1176-sdk/middleware/sdmmc/common/fsl_sdmmc_common.h"

void BOARD_SDIO_Config(void *card, sd_cd_t cd, uint32_t hostIRQPriority, sdio_int_t cardInt);

#endif  // _LIBS_NXP_RT1176_SDK_SDMMC_CONFIG_H_
