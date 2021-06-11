#ifndef _LIBS_BASE_GPIO_H_
#define _LIBS_BASE_GPIO_H_

#include <functional>

namespace valiant {
namespace gpio {

enum Gpio {
    kPowerLED,
    kUserLED,
    kEdgeTpuPgood,
    kEdgeTpuReset,
    kEdgeTpuPmic,
    kBtRegOn,
    kUserButton,
    kCameraTrigger,
    kAntennaSelect,
    kCount
};
using GpioCallback = std::function<void()>;

void Init();
void SetGpio(Gpio gpio, bool enable);
bool GetGpio(Gpio gpio);
void SetMode(Gpio gpio, bool input, bool pull, bool pull_direction);
void RegisterIRQHandler(Gpio gpio, GpioCallback cb);

}  // namespace gpio
}  // namespace valiant

#endif  // _LIBS_BASE_GPIO_H_