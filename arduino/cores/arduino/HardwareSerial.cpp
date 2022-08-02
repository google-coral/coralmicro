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

#include <HardwareSerial.h>

#include <cassert>

#include "libs/base/console_m7.h"

namespace coralmicro {
namespace arduino {

void HardwareSerial::begin(unsigned long baudrate) {
  assert(baudrate == 115200);
}

void HardwareSerial::begin(unsigned long baudrate, uint16_t config) {
  begin(baudrate);
}

void HardwareSerial::end() {}

int HardwareSerial::available() {
  return static_cast<int>(coralmicro::ConsoleM7::GetSingleton()->available());
}

int HardwareSerial::peek() {
  return static_cast<int>(coralmicro::ConsoleM7::GetSingleton()->peek());
}

int HardwareSerial::read() {
  char ch;
  int ret = coralmicro::ConsoleM7::GetSingleton()->Read(&ch, 1);
  if (ret == -1) return -1;
  return static_cast<int>(ch);
}

void HardwareSerial::flush() {}

size_t HardwareSerial::write(uint8_t c) {
  coralmicro::ConsoleM7::GetSingleton()->Write(reinterpret_cast<char*>(&c), 1);
  return 1;
}

HardwareSerial::operator bool() { return true; }

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::HardwareSerial Serial;
