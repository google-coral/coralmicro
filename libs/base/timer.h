#ifndef _LIBS_BASE_TIMER_H_
#define _LIBS_BASE_TIMER_H_

#include <cstdint>

namespace valiant {
namespace timer {

void Init();

// Milliseconds since boot
uint32_t millis();

// Microseconds since boot
uint32_t micros();

}  // namespace timer
}  // namespace valiant

#endif  // _LIBS_BASE_TIMER_H_