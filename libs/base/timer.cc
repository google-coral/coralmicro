#include "libs/base/timer.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpt.h"

#include <climits>

namespace coral::micro {
namespace timer {

// Number of times the microseconds counter has rolled over.
static uint32_t micros_rollover = 0;

extern "C" void GPT1_IRQHandler(void) {
    if (GPT_GetStatusFlags(GPT1, kGPT_RollOverFlag)) {
        GPT_ClearStatusFlags(GPT1, kGPT_RollOverFlag);
        micros_rollover++;
    }
}

void Init() {
    gpt_config_t gpt_config;
    GPT_GetDefaultConfig(&gpt_config);
    gpt_config.clockSource = kGPT_ClockSource_Periph;


    uint32_t root_freq = CLOCK_GetRootClockFreq(kCLOCK_Root_Gpt1);

    GPT_Init(GPT1, &gpt_config);
    GPT_EnableInterrupts(GPT1, kGPT_RollOverFlagInterruptEnable);
    GPT_ClearStatusFlags(GPT1, kGPT_RollOverFlag);
    // Set the divider to get us close to 1MHz.
    GPT_SetClockDivider(GPT1, root_freq / 1000000);
    EnableIRQ(GPT1_IRQn);

    GPT_StartTimer(GPT1);
}

uint32_t millis() {
    return (micros_rollover * (UINT_MAX / 1000)) + (GPT_GetCurrentTimerCount(GPT1) / 1000);
}

uint32_t micros() {
    return GPT_GetCurrentTimerCount(GPT1);
}

}  // namespace timer
}  // namespace coral::micro

extern "C" uint32_t vPortGetRunTimeCounterValue(void) {
    return coral::micro::timer::micros();
}
