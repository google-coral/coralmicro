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
    case A0:
      return coralmicro::Gpio::kArduinoA0;
    case A1:
      return coralmicro::Gpio::kArduinoA1;
    case A3:
      return coralmicro::Gpio::kArduinoA3;
    case A4:
      return coralmicro::Gpio::kArduinoA4;
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
      coralmicro::GpioSetMode(gpio, coralmicro::GpioMode::kInput);
      break;
    case OUTPUT:
      coralmicro::GpioSetMode(gpio, coralmicro::GpioMode::kOutput);
      break;
    case INPUT_PULLUP:
      coralmicro::GpioSetMode(gpio, coralmicro::GpioMode::kInputPullUp);
      break;
    case INPUT_PULLDOWN:
      coralmicro::GpioSetMode(gpio, coralmicro::GpioMode::kInputPullDown);
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
  coralmicro::GpioConfigureInterrupt(gpio, interrupt_mode, callback);
}

void detachInterrupt(pin_size_t interruptNumber) {
  coralmicro::Gpio gpio = PinNumberToGpio(interruptNumber);
  coralmicro::GpioConfigureInterrupt(
      gpio, coralmicro::GpioInterruptMode::kIntModeNone, []() {});
}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
  switch (pin) {
    case PIN_BTN:
    case D0:
    case D1:
    case D2:
    case D3: {
      auto start_time = coralmicro::TimerMicros();
      while (digitalRead(pin) != state) {
        auto duration = coralmicro::TimerMicros() - start_time;
        if (duration >= timeout) {
          return 0;  // Timeout is reached, pulse did not start.
        }
      }
      auto stop_state = state == LOW ? HIGH : LOW;
      auto start_pulse_time = coralmicro::TimerMicros();
      while (digitalRead(pin) != stop_state) {
        auto duration = coralmicro::TimerMicros() - start_time;
        if (duration >= timeout) {
          return 0;  // Timeout is reached, pulse not complete.
        }
      }
      return coralmicro::TimerMicros() - start_pulse_time;
    }
    default:
      assert(false);
  }
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
  return pulseIn(pin, state, timeout);
}

uint8_t shiftIn(pin_size_t dataPin, pin_size_t clockPin, BitOrder bitOrder) {
  uint8_t value = 0;
  uint8_t i;

  for (i = 0; i < 8; ++i) {
    digitalWrite(clockPin, HIGH);
    if (bitOrder == LSBFIRST)
      value |= digitalRead(dataPin) << i;
    else  // MSBFIRST
      value |= digitalRead(dataPin) << (7 - i);
    digitalWrite(clockPin, LOW);
  }
  return value;
}

void shiftOut(pin_size_t dataPin, pin_size_t clockPin, BitOrder bitOrder,
              uint8_t val) {
  uint8_t i;

  for (i = 0; i < 8; i++) {
    if (bitOrder == LSBFIRST) {
      digitalWrite(dataPin, (val & 1) ? HIGH : LOW);
      val >>= 1;
    } else {  // MSBFIRST
      digitalWrite(dataPin, ((val & 128) != 0) ? HIGH : LOW);
      val <<= 1;
    }

    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
}
