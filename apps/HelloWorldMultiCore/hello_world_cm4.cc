#include "third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"

extern "C" void main_task(osa_task_param_t arg) {
    uint32_t startup_data;
    mcmgr_status_t status;
    do {
        status = MCMGR_GetStartupData(&startup_data);
    } while (status != kStatus_MCMGR_Success);
    while(true);
}
