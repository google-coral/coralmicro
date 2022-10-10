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

#ifndef LIBS_BASE_MUTEX_H_
#define LIBS_BASE_MUTEX_H_

#include "libs/base/check.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_sema4.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"

namespace coralmicro {
// Defines a mutex lock for the active MCU core, ensuring safe handling of
// any resources that are shared between tasks.
class MutexLock {
 public:
  // Acquires a mutex lock using a semaphore.
  //
  // Any code following this request for the mutex lock is blocked until the
  // lock is successfully acquired, and each unique mutex lock can be held by
  // only one task at a time (on the same MCU core).
  // Thus, code-blocks that use the same semaphore to get a `MutexLock` (on the
  // same MCU core) will have thread-safe variables between said code-blocks.
  //
  // After execution leaves the code block where the lock is acquired, the mutex
  // lock is automatically released.
  //
  // @param sema The [SemaphoreHandle](https://www.freertos.org/xSemaphoreCreateBinary.html)
  //   to define a unique mutex lock.
  explicit MutexLock(SemaphoreHandle_t sema) : sema_(sema) {
    CHECK(xSemaphoreTake(sema_, portMAX_DELAY) == pdTRUE);
  }
  // @cond
  ~MutexLock() { CHECK(xSemaphoreGive(sema_) == pdTRUE); }

  MutexLock(const MutexLock&) = delete;
  MutexLock& operator=(const MutexLock&) = delete;
  // @endcond

 private:
  SemaphoreHandle_t sema_;
};

// Defines a mutex lock that is unique across MCU cores, ensuring safe handling
// of any resources that are shared between the M7 and M4 cores.
class MulticoreMutexLock {
 public:
  // Acquires a multi-core mutex lock using a semaphore gate number.
  //
  // Any code following this request for the mutex lock is blocked until the
  // lock is successfully acquired, and each unique mutex lock can be held by
  // only one task at a time, regardless of which MCU core the task is on.
  // Thus, code-blocks that use the same gate to get a `MulticoreMutexLock`
  // will have thread-safe variables, even across cores.
  //
  // After execution leaves the code block where the lock is acquired, the mutex
  // lock is automatically released.
  //
  // @param gate An integer representing a unique semaphore gate.
  // Must be within range of the maximum number of gates available on the
  // hardware (16).
  explicit MulticoreMutexLock(uint8_t gate) : gate_(gate) {
    assert(gate < SEMA4_GATE_COUNT);
#if (__CORTEX_M == 7)
    core_ = 0;
#elif (__CORTEX_M == 4)
    core_ = 1;
#else
#error "Unknown __CORTEX_M"
#endif
    SEMA4_Lock(SEMA4, gate_, core_);
  }
  // @cond
  ~MulticoreMutexLock() { SEMA4_Unlock(SEMA4, gate_); }

  MulticoreMutexLock(const MutexLock&) = delete;
  MulticoreMutexLock& operator=(const MutexLock&) = delete;
  // @endcond

 private:
  uint8_t gate_;
  uint8_t core_;
};

}  // namespace coralmicro

#endif  // LIBS_BASE_MUTEX_H_
