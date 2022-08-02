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

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <api/HardwareSerial.h>

namespace coralmicro {
namespace arduino {

class HardwareSerial : public ::arduino::HardwareSerial {
 public:
  // TODO(atv): Currently, this does not configure pins: it assumes they are
  // already set to work as serial.
  void begin(unsigned long baudrate);
  void begin(unsigned long baudrate, uint16_t config);
  // TODO(atv): This does not return pins to GPIO mode.
  void end();
  int available();
  int peek();
  int read();
  void flush();
  size_t write(uint8_t c);
  using ::arduino::Print::write;
  operator bool();
};

}  // namespace arduino
}  // namespace coralmicro

#endif  // HardwareSerial_h
