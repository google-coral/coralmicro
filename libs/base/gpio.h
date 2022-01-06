#ifndef _LIBS_BASE_GPIO_H_
#define _LIBS_BASE_GPIO_H_

#include <functional>

namespace valiant {
namespace gpio {

enum Gpio {
#if defined(VALIANT_ARDUINO) && (VALIANT_ARDUINO == 1)
    kArduinoD0,
    kArduinoD1,
    kArduinoD2,
    kArduinoD3,
#endif
    kPowerLED,
    kUserLED,
    kTpuLED,
    kEdgeTpuPgood,
    kEdgeTpuReset,
    kEdgeTpuPmic,
    kBtRegOn,
    kUserButton,
    kCameraTrigger,
    kAntennaSelect,
    kBtHostWake,
    kBtDevWake,
    kEthPhyRst,
    kBufferEnable,
    kCameraPrivacyOverride,
    kCount
};

enum InterruptMode {
    kIntModeNone,
    kIntModeLow,
    kIntModeHigh,
    kIntModeRising,
    kIntModeFalling,
    kIntModeChanging,
    kIntModeCount
};

using GpioCallback = std::function<void()>;

void Init();
void SetGpio(Gpio gpio, bool enable);
bool GetGpio(Gpio gpio);
void SetMode(Gpio gpio, bool input, bool pull, bool pull_direction);
void SetIntMode(Gpio gpio, InterruptMode mode);
void RegisterIRQHandler(Gpio gpio, GpioCallback cb);

}  // namespace gpio
}  // namespace valiant

#endif  // _LIBS_BASE_GPIO_H_
