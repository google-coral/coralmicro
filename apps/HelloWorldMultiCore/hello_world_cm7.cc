#include "third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"
#include <cstdio>

#define CORE1_BOOT_ADDRESS 0x20200000

extern char _binary_HelloWorldMultiCoreM4_bin_size;
extern char _binary_HelloWorldMultiCoreM4_bin_start;

extern "C" void main_task(osa_task_param_t arg) {
    printf("Hello world from M7.\r\n");
    uint32_t m4_start = (uint32_t)&_binary_HelloWorldMultiCoreM4_bin_start;
    uint32_t m4_size = (uint32_t)&_binary_HelloWorldMultiCoreM4_bin_size;
    printf("CM4 binary is %lu bytes at 0x%lx\r\n", m4_size, m4_start);

    memcpy((void*)CORE1_BOOT_ADDRESS, (void*)m4_start, m4_size);
    printf("Starting M4\r\n");
    MCMGR_StartCore(kMCMGR_Core1, (void*)CORE1_BOOT_ADDRESS, 0, kMCMGR_Start_Synchronous);
    printf("M4 started\r\n");

    while(true);
}
