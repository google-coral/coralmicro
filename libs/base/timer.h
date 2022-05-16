#ifndef LIBS_BASE_TIMER_H_
#define LIBS_BASE_TIMER_H_

#include <cstdint>

namespace coral::micro {
namespace timer {

void Init();

// Milliseconds since boot
uint32_t millis();

// Microseconds since boot
uint32_t micros();

}  // namespace timer
}  // namespace coral::micro

#endif  // LIBS_BASE_TIMER_H_
