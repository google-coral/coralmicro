#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <api/HardwareSerial.h>

namespace valiant {
namespace arduino {

class HardwareSerial : public ::arduino::HardwareSerial {
  public:
    // TODO(atv): Currently, this does not configure pins: it assumes they are already
    // set to work as serial.
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
}  // namespace valiant

#endif  // HardwareSerial_h