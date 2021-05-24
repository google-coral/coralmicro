#ifndef _LIBS_BASE_GPIO_H_
#define _LIBS_BASE_GPIO_H_

namespace valiant {
namespace gpio {

enum Gpio {
    kPowerLED,
    kUserLED,
    kEdgeTpuPgood,
    kEdgeTpuReset,
    kEdgeTpuPmic,
    kBtRegOn,
    kCount
};

void Init();
void SetGpio(Gpio gpio, bool enable);
bool GetGpio(Gpio gpio);

}  // namespace gpio
}  // namespace valiant

#endif  // _LIBS_BASE_GPIO_H_