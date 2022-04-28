#include "libs/base/led.h"

#include "libs/base/gpio.h"
#include "libs/base/pwm.h"

namespace coral::micro {
namespace led {

bool Set(LED led, bool enable) {
    return Set(led, enable, enable ? kFullyOn : kFullyOff);
}

bool Set(LED led, bool enable, unsigned int brightness) {
    bool ret = true;
    switch (led) {
        case LED::kPower:
            gpio::SetGpio(gpio::Gpio::kPowerLED, enable);
            break;
        case LED::kUser:
            gpio::SetGpio(gpio::Gpio::kUserLED, enable);
            break;
        case LED::kTpu:
#if __CORTEX_M == 7
            if (!gpio::GetGpio(gpio::Gpio::kEdgeTpuPmic)) {
                printf("TPU LED requires TPU power to be enabled.\r\n");
                ret = false;
                break;
            }
            if (brightness > kFullyOn) {
                brightness = kFullyOff;
            }
            pwm::PwmModuleConfig config;
            config.base = PWM1;
            config.module = kPWM_Module_1;
            if (enable) {
                config.A.enabled = true;
                config.A.duty_cycle = brightness;
                config.B.enabled = false;
                pwm::Init(config);
                pwm::Enable(config, true);
            } else {
                pwm::Enable(config, false);
            }
#else
            ret = false;
#endif
            break;
        default:
            ret = false;
    }
    return ret;
}

}  // namespace led
}  // namespace coral::micro
