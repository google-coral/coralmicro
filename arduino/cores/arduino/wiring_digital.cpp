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
#include "libs/base/timer.h"
#include "pins_arduino.h"

static coralmicro::Gpio PinNumberToGpio(pin_size_t pinNumber) {
    switch (pinNumber) {
        case PIN_LED_USER:
            return coralmicro::Gpio::kUserLed;
        case PIN_BTN:
            return coralmicro::Gpio::kUserButton;
        case PIN_LED_STATUS:
            return coralmicro::Gpio::kStatusLed;
        case D0:
            return coralmicro::Gpio::kArduinoD0;
        case D1:
            return coralmicro::Gpio::kArduinoD1;
        case D2:
            return coralmicro::Gpio::kArduinoD2;
        case D3:
            return coralmicro::Gpio::kArduinoD3;
        default:
            assert(false);
            return coralmicro::Gpio::kCount;
    }
}

static coralmicro::GpioInterruptMode PinStatusToInterruptMode(PinStatus mode) {
    switch (mode) {
        case HIGH:
            return coralmicro::GpioInterruptMode::kIntModeHigh;
        case LOW:
            return coralmicro::GpioInterruptMode::kIntModeLow;
        case CHANGE:
            return coralmicro::GpioInterruptMode::kIntModeChanging;
        case RISING:
            return coralmicro::GpioInterruptMode::kIntModeRising;
        case FALLING:
            return coralmicro::GpioInterruptMode::kIntModeFalling;
        default:
            assert(false);
            return coralmicro::GpioInterruptMode::kIntModeCount;
    }
}

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
    coralmicro::Gpio gpio = PinNumberToGpio(pinNumber);
    switch (pinMode) {
        case INPUT:
            coralmicro::GpioSetMode(gpio, true, coralmicro::GpioPullDirection::kNone);
            break;
        case OUTPUT:
            coralmicro::GpioSetMode(gpio, false, coralmicro::GpioPullDirection::kNone);
            break;
        case INPUT_PULLUP:
            coralmicro::GpioSetMode(gpio, true, coralmicro::GpioPullDirection::kPullUp);
            break;
        case INPUT_PULLDOWN:
            coralmicro::GpioSetMode(gpio, true, coralmicro::GpioPullDirection::kPullDown);
            break;
    }
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
    assert(status == LOW || status == HIGH);
    coralmicro::Gpio gpio = PinNumberToGpio(pinNumber);
    coralmicro::GpioSet(gpio, status == LOW ? false : true);
}

PinStatus digitalRead(pin_size_t pinNumber) {
    coralmicro::Gpio gpio = PinNumberToGpio(pinNumber);
    bool status = coralmicro::GpioGet(gpio);
    return status ? HIGH : LOW;
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback,
                     PinStatus mode) {
    coralmicro::Gpio gpio = PinNumberToGpio(interruptNumber);
    auto interrupt_mode = PinStatusToInterruptMode(mode);
    coralmicro::GpioSetIntMode(gpio, interrupt_mode);
    coralmicro::GpioRegisterIrqHandler(gpio, callback);
}

void detachInterrupt(pin_size_t interruptNumber) {
    coralmicro::Gpio gpio = PinNumberToGpio(interruptNumber);
    coralmicro::GpioRegisterIrqHandler(gpio, nullptr);
    coralmicro::GpioSetIntMode(gpio,
                               coralmicro::GpioInterruptMode::kIntModeNone);
}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    switch (pin) {
        case PIN_BTN:
        case D0:
        case D1:
        case D2:
        case D3: {
            delayMicroseconds(timeout);
            auto stop_state = state == LOW ? HIGH : LOW;
            while (digitalRead(pin) != state) {
            }
            auto start_time = coralmicro::TimerMicros();
            while (digitalRead(pin) != stop_state) {
            }
            return coralmicro::TimerMicros() - start_time;
        }
        default:
            assert(false);
    }
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
    return pulseIn(pin, state, timeout);
}
