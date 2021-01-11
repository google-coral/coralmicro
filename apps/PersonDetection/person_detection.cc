#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/main_functions.h"

#include <cstdio>
#include <cstring>

extern "C" void DebugLog(const char *s) {
#if __CORTEX_M == 7
    printf("[M7] %s", s);
#else
    printf("[M4] %s", s);
#endif
}

extern "C" void app_main(void *param) {
    setup();
    DebugLog("PersonDetection\r\n");
    while (true) {
        loop();
        taskYIELD();
    }
}
