#include "third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction.h"
#include <cstdio>

extern "C" void main_task(osa_task_param_t arg) {
    printf("Hello world.\r\n");
    while(true);
}
