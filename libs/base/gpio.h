#ifndef LIBS_BASE_GPIO_H_
#define LIBS_BASE_GPIO_H_

#include <functional>

namespace coral::micro {
namespace gpio {

enum Gpio {
#if defined(CORAL_MICRO_ARDUINO) && (CORAL_MICRO_ARDUINO == 1)
    kArduinoD0,
    kArduinoD1,
    kArduinoD2,
    kArduinoD3,
#endif
    kPowerLED,
    kUserLED,
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
    kCryptoRst,
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
}  // namespace coral::micro

#endif  // LIBS_BASE_GPIO_H_
