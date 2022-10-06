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

#ifndef LIBS_BASE_TIMER_H_
#define LIBS_BASE_TIMER_H_

#include <cstdint>
#include <ctime>

namespace coralmicro {

// Initializes the timer.
//
// Programs on the M7 do not need to call this because it is automatically
// called internally. M7 programs can immediately call functions such as
// `TimerMicros()`.
//
// Programs on the M4 must call this to intialize the timer before they can
// use timers. For example:
//
// ```
// TimerInit();
// auto current_time = TimerMillis();
// ```
// Beware that this resets the timer's clock each time you call it.
void TimerInit();

// Microseconds since boot.
uint64_t TimerMicros();

// Milliseconds since boot.
inline uint64_t TimerMillis() { return TimerMicros() / 1000; }

void TimerSetRtcTime(uint32_t sec);
void TimerGetRtcTime(struct tm* time);

}  // namespace coralmicro

#endif  // LIBS_BASE_TIMER_H_
