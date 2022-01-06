#include "Arduino.h"
#include "pins_arduino.h"

#include "libs/base/gpio.h"

#include <cassert>

static valiant::gpio::Gpio PinNumberToGpio(pin_size_t pinNumber) {
    switch (pinNumber) {
        case PIN_LED:
            return valiant::gpio::Gpio::kUserLED;
        case PIN_BTN:
            return valiant::gpio::Gpio::kUserButton;
        case D0:
            return valiant::gpio::Gpio::kArduinoD0;
        case D1:
            return valiant::gpio::Gpio::kArduinoD1;
        case D2:
            return valiant::gpio::Gpio::kArduinoD2;
        case D3:
            return valiant::gpio::Gpio::kArduinoD3;
        default:
            assert(false);
            return valiant::gpio::Gpio::kCount;
    }
}

static valiant::gpio::InterruptMode PinStatusToInterruptMode(PinStatus mode) {
    switch (mode) {
        case HIGH:
            return valiant::gpio::kIntModeHigh;
        case LOW:
            return valiant::gpio::kIntModeLow;
        case CHANGE:
            return valiant::gpio::kIntModeChanging;
        case RISING:
            return valiant::gpio::kIntModeRising;
        case FALLING:
            return valiant::gpio::kIntModeFalling;
        default:
            assert(false);
            return valiant::gpio::kIntModeCount;
    }
}


void pinMode(pin_size_t pinNumber, PinMode pinMode) {
    valiant::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    switch (pinMode) {
        case INPUT:
            valiant::gpio::SetMode(gpio, true, false, false);
            break;
        case OUTPUT:
            valiant::gpio::SetMode(gpio, false, false, false);
            break;
        case INPUT_PULLUP:
            valiant::gpio::SetMode(gpio, true, true, true);
            break;
        case INPUT_PULLDOWN:
            valiant::gpio::SetMode(gpio, true, true, false);
            break;
        break;
    }
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
    assert(status == LOW || status == HIGH);
    valiant::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    valiant::gpio::SetGpio(gpio, status == LOW ? false : true);
}

PinStatus digitalRead(pin_size_t pinNumber) {
    valiant::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    bool status = valiant::gpio::GetGpio(gpio);
    return status ? HIGH : LOW;
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode) {
    valiant::gpio::Gpio gpio = PinNumberToGpio(interruptNumber);
    auto interrupt_mode = PinStatusToInterruptMode(mode);
    valiant::gpio::SetIntMode(gpio, interrupt_mode);
    valiant::gpio::RegisterIRQHandler(gpio, callback);
}

void detachInterrupt(pin_size_t interruptNumber) {
    valiant::gpio::Gpio gpio = PinNumberToGpio(interruptNumber);
    valiant::gpio::RegisterIRQHandler(gpio, nullptr);
    valiant::gpio::SetIntMode(gpio, valiant::gpio::kIntModeNone);
}
