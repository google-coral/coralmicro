#include "libs/base/analog.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <cstdio>

extern "C" void app_main(void *param) {
    coral::micro::analog::Init(coral::micro::analog::Device::ADC1);
    coral::micro::analog::Init(coral::micro::analog::Device::DAC1);
    coral::micro::analog::ADCConfig config;
    coral::micro::analog::CreateConfig(
        config,
        coral::micro::analog::Device::ADC1, 0,
        coral::micro::analog::Side::B,
        false
    );
    coral::micro::analog::EnableDAC(true);
    while (true) {
        uint16_t val = coral::micro::analog::ReadADC(config);
        coral::micro::analog::WriteDAC(val);
        printf("ADC val: %u\r\n", val);
    }
}
