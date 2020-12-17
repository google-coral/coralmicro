#include "third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/romapi/fsl_romapi.h"
#include <cstdio>



extern "C" void main_task(osa_task_param_t arg) {
    uint32_t boot_arg = 0xeb100000;
    printf("BootROM\r\n");
    ROM_API_Init();
    ROM_RunBootloader(&boot_arg);
    while(true);
}
