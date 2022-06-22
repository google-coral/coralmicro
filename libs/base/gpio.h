#ifndef LIBS_BASE_GPIO_H_
#define LIBS_BASE_GPIO_H_

#include <functional>

namespace coral::micro {
namespace gpio {

// Enumeration of the pre-configured GPIO pins.
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
    kBtHostWake,
    kBtDevWake,
    kEthPhyRst,
    kCameraPrivacyOverride,
    kCryptoRst,
    kSpiCs,
    kSpiSck,
    kSpiSdo,
    kSpiSdi,
    kSda6,
    kScl1,
    kSda1,
    kAA,
    kAB,
    kUartCts,
    kUartRts,
    kPwm1,
    kPwm0,
    kScl6,
    kCount,

    kArduinoD0 = kScl6,
    kArduinoD1 = kUartRts,
    kArduinoD2 = kUartCts,
    kArduinoD3 = kSda6,
};

// Enumeration of available interrupt modes for GPIOs.
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

// @cond Internal only, do not generate docs
void Init();
// @endcond

// Sets the output value of a GPIO.
// @param gpio Pin to configure. Only pins in the `Gpio` enumeration can be configured with this module.
//             To use a GPIO that is not covered by this module,
//             use the functions in `third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.h`
// @param enable Whether to set the pin to high or low.
void SetGpio(Gpio gpio, bool enable);

// Gets the input value of a GPIO.
// @param gpio Pin to read.
// @returns Boolean representing high or low state of the pin.
bool GetGpio(Gpio gpio);

// Sets the mode of a GPIO.
// @param gpio Pin to configure.
// @param input True sets the pin as an input; false sets it as an output.
// @param pull True enables the pin to have a pull; false disables it.
// @param pull_direction True enables pull up; false enables pull down. This is ignored if `pull` is false.
void SetMode(Gpio gpio, bool input, bool pull, bool pull_direction);

// Sets the interrupt mode of a GPIO.
// @param gpio Pin to configure.
// @param mode The style of interrupt to sense.
void SetIntMode(Gpio gpio, InterruptMode mode);

// Register an interrupt handler for a GPIO.
// @param Pin for which to register the handler.
// @param cb Callback function that will be invoked when the interrupt is raised.
//           This is called from interrupt context, so it should not do much work.
void RegisterIRQHandler(Gpio gpio, GpioCallback cb);

}  // namespace gpio
}  // namespace coral::micro

#endif  // LIBS_BASE_GPIO_H_
