#include <HardwareSerial.h>

#include "libs/base/console_m7.h"

#include <cassert>

namespace valiant {
namespace arduino {

void HardwareSerial::begin(unsigned long baudrate) {
    assert(baudrate == 115200);
}

void HardwareSerial::begin(unsigned long baudrate, uint16_t config) {
    begin(baudrate);
}

void HardwareSerial::end() {
}

int HardwareSerial::available() {
    return static_cast<int>(valiant::ConsoleM7::GetSingleton()->available());
}

int HardwareSerial::peek() {
    return static_cast<int>(valiant::ConsoleM7::GetSingleton()->peek());
}

int HardwareSerial::read() {
    char ch;
    int ret = valiant::ConsoleM7::GetSingleton()->Read(&ch, 1);
    if (ret == -1)
        return -1;
    return static_cast<int>(ch);
}

void HardwareSerial::flush() {
}

size_t HardwareSerial::write(uint8_t c) {
    valiant::ConsoleM7::GetSingleton()->Write(reinterpret_cast<char*>(&c), 1);
    return 1;
}

HardwareSerial::operator bool() {
    return true;
}

}  // namespace arduino
}  // namespace valiant

valiant::arduino::HardwareSerial Serial;