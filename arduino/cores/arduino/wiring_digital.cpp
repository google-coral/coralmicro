/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>

#include "Arduino.h"
#include "libs/base/gpio.h"
#include "pins_arduino.h"

static coral::micro::gpio::Gpio PinNumberToGpio(pin_size_t pinNumber) {
    switch (pinNumber) {
        case PIN_LED_USER:
            return coral::micro::gpio::Gpio::kUserLED;
        case PIN_BTN:
            return coral::micro::gpio::Gpio::kUserButton;
        case PIN_LED_STATUS:
            return coral::micro::gpio::Gpio::kStatusLED;
        case D0:
            return coral::micro::gpio::Gpio::kArduinoD0;
        case D1:
            return coral::micro::gpio::Gpio::kArduinoD1;
        case D2:
            return coral::micro::gpio::Gpio::kArduinoD2;
        case D3:
            return coral::micro::gpio::Gpio::kArduinoD3;
        default:
            assert(false);
            return coral::micro::gpio::Gpio::kCount;
    }
}

static coral::micro::gpio::InterruptMode PinStatusToInterruptMode(PinStatus mode) {
    switch (mode) {
        case HIGH:
            return coral::micro::gpio::kIntModeHigh;
        case LOW:
            return coral::micro::gpio::kIntModeLow;
        case CHANGE:
            return coral::micro::gpio::kIntModeChanging;
        case RISING:
            return coral::micro::gpio::kIntModeRising;
        case FALLING:
            return coral::micro::gpio::kIntModeFalling;
        default:
            assert(false);
            return coral::micro::gpio::kIntModeCount;
    }
}

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
    coral::micro::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    switch (pinMode) {
        case INPUT:
            coral::micro::gpio::SetMode(gpio, true, false, false);
            break;
        case OUTPUT:
            coral::micro::gpio::SetMode(gpio, false, false, false);
            break;
        case INPUT_PULLUP:
            coral::micro::gpio::SetMode(gpio, true, true, true);
            break;
        case INPUT_PULLDOWN:
            coral::micro::gpio::SetMode(gpio, true, true, false);
            break;
    }
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
    assert(status == LOW || status == HIGH);
    coral::micro::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    coral::micro::gpio::SetGpio(gpio, status == LOW ? false : true);
}

PinStatus digitalRead(pin_size_t pinNumber) {
    coral::micro::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    bool status = coral::micro::gpio::GetGpio(gpio);
    return status ? HIGH : LOW;
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback,
                     PinStatus mode) {
    coral::micro::gpio::Gpio gpio = PinNumberToGpio(interruptNumber);
    auto interrupt_mode = PinStatusToInterruptMode(mode);
    coral::micro::gpio::SetIntMode(gpio, interrupt_mode);
    coral::micro::gpio::RegisterIRQHandler(gpio, callback);
}

void detachInterrupt(pin_size_t interruptNumber) {
    coral::micro::gpio::Gpio gpio = PinNumberToGpio(interruptNumber);
    coral::micro::gpio::RegisterIRQHandler(gpio, nullptr);
    coral::micro::gpio::SetIntMode(gpio, coral::micro::gpio::kIntModeNone);
}
