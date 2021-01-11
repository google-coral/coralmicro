#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"
#include <cstdio>
#include <cstring>

#define CORE1_BOOT_ADDRESS 0x20200000

extern char m4_binary_size;
extern char m4_binary_start;

extern "C" void app_main(void *param) {
    printf("Hello world from M7.\r\n");
    uint32_t m4_start = (uint32_t)&m4_binary_start;
    uint32_t m4_size = (uint32_t)&m4_binary_size;
    printf("CM4 binary is %lu bytes at 0x%lx\r\n", m4_size, m4_start);

    memcpy((void*)CORE1_BOOT_ADDRESS, (void*)m4_start, m4_size);
    printf("Starting M4\r\n");
    MCMGR_StartCore(kMCMGR_Core1, (void*)CORE1_BOOT_ADDRESS, 0, kMCMGR_Start_Synchronous);
    printf("M4 started\r\n");

    while(true);
}
