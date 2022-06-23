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
// Defines a mutex lock over a given semaphore.
// Any code in the code block following the mutex lock will only be processed after the semaphore is acquired.
// Thus, code-blocks that use a mutex lock to acquire the same semaphore will result in thread-safe variables between said code-blocks.
// After leaving the code-block the mutex lock releases the semaphore and the mutex-lock is deleted.

class MutexLock {
   public:
    // @param sema SemaphoreHandle to be acquired.
    explicit MutexLock(SemaphoreHandle_t sema) : sema_(sema) {
        CHECK(xSemaphoreTake(sema_, portMAX_DELAY) == pdTRUE);
    }

    ~MutexLock() { CHECK(xSemaphoreGive(sema_) == pdTRUE); }

    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;

   private:
    SemaphoreHandle_t sema_;
};

// Defines a core specific mutex lock over a given semaphore gate.
// Because each core has its own copy of semaphore gates,
// two mutex locks operating on separate cores can acquire the same gate simultaneously.
// This results in being able to use a multicore mutex lock to represent hardware resources specific to each core.
// Thus, you can use multicore locks to partition hardware resources to concurrently run processes on different cores.
class MulticoreMutexLock {
  public:
   // @param gate Unsigned int representing the semaphore gate.
   // Must be within range of the maximum number of gates available on the  hardware (16).
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
    ~MulticoreMutexLock() { SEMA4_Unlock(SEMA4, gate_); }

    MulticoreMutexLock(const MutexLock&) = delete;
    MulticoreMutexLock& operator=(const MutexLock&) = delete;

   private:
    uint8_t gate_;
    uint8_t core_;
};

}  // namespace coralmicro

#endif  // LIBS_BASE_MUTEX_H_
