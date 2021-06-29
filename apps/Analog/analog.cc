#include "libs/base/analog.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <cstdio>

extern "C" void app_main(void *param) {
    valiant::analog::Init(valiant::analog::Device::ADC1);
    valiant::analog::ADCConfig config;
    valiant::analog::CreateConfig(
        config,
        valiant::analog::Device::ADC1, 0,
        valiant::analog::Side::B,
        false
    );
    valiant::analog::EnableDAC(true);
    while (true) {
        uint16_t val = valiant::analog::ReadADC(config);
        valiant::analog::WriteDAC(val);
        printf("ADC val: %u\r\n", val);
    }
}