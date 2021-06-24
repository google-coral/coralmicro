#include "Arduino.h"
#include "wiring_private.h"

#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.h"

void wiringInit() {
    wiringAnalogInit();
}

void delay(unsigned long ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void delayMicroseconds(unsigned int us) {
    SDK_DelayAtLeastUs(us, CLOCK_GetFreq(kCLOCK_CpuClk));
}

unsigned long millis() {
    return valiant::timer::millis();
}

unsigned long micros() {
    return valiant::timer::micros();
}