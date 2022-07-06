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

static coralmicro::gpio::Gpio PinNumberToGpio(pin_size_t pinNumber) {
    switch (pinNumber) {
        case PIN_LED_USER:
            return coralmicro::gpio::Gpio::kUserLED;
        case PIN_BTN:
            return coralmicro::gpio::Gpio::kUserButton;
        case PIN_LED_STATUS:
            return coralmicro::gpio::Gpio::kStatusLED;
        case D0:
            return coralmicro::gpio::Gpio::kArduinoD0;
        case D1:
            return coralmicro::gpio::Gpio::kArduinoD1;
        case D2:
            return coralmicro::gpio::Gpio::kArduinoD2;
        case D3:
            return coralmicro::gpio::Gpio::kArduinoD3;
        default:
            assert(false);
            return coralmicro::gpio::Gpio::kCount;
    }
}

static coralmicro::gpio::InterruptMode PinStatusToInterruptMode(PinStatus mode) {
    switch (mode) {
        case HIGH:
            return coralmicro::gpio::kIntModeHigh;
        case LOW:
            return coralmicro::gpio::kIntModeLow;
        case CHANGE:
            return coralmicro::gpio::kIntModeChanging;
        case RISING:
            return coralmicro::gpio::kIntModeRising;
        case FALLING:
            return coralmicro::gpio::kIntModeFalling;
        default:
            assert(false);
            return coralmicro::gpio::kIntModeCount;
    }
}

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
    coralmicro::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    switch (pinMode) {
        case INPUT:
            coralmicro::gpio::SetMode(gpio, true, false, false);
            break;
        case OUTPUT:
            coralmicro::gpio::SetMode(gpio, false, false, false);
            break;
        case INPUT_PULLUP:
            coralmicro::gpio::SetMode(gpio, true, true, true);
            break;
        case INPUT_PULLDOWN:
            coralmicro::gpio::SetMode(gpio, true, true, false);
            break;
    }
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
    assert(status == LOW || status == HIGH);
    coralmicro::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    coralmicro::gpio::SetGpio(gpio, status == LOW ? false : true);
}

PinStatus digitalRead(pin_size_t pinNumber) {
    coralmicro::gpio::Gpio gpio = PinNumberToGpio(pinNumber);
    bool status = coralmicro::gpio::GetGpio(gpio);
    return status ? HIGH : LOW;
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback,
                     PinStatus mode) {
    coralmicro::gpio::Gpio gpio = PinNumberToGpio(interruptNumber);
    auto interrupt_mode = PinStatusToInterruptMode(mode);
    coralmicro::gpio::SetIntMode(gpio, interrupt_mode);
    coralmicro::gpio::RegisterIRQHandler(gpio, callback);
}

void detachInterrupt(pin_size_t interruptNumber) {
    coralmicro::gpio::Gpio gpio = PinNumberToGpio(interruptNumber);
    coralmicro::gpio::RegisterIRQHandler(gpio, nullptr);
    coralmicro::gpio::SetIntMode(gpio, coralmicro::gpio::kIntModeNone);
}
