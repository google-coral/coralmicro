#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"

extern "C" void app_main(void *param) {
    uint32_t startup_data;
    mcmgr_status_t status;
    do {
        status = MCMGR_GetStartupData(&startup_data);
    } while (status != kStatus_MCMGR_Success);
    while(true);
}
