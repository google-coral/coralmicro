#ifndef _LIBS_BASE_MUTEX_H_
#define _LIBS_BASE_MUTEX_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_sema4.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"

namespace coral::micro {

class MutexLock {
   public:
    explicit MutexLock(SemaphoreHandle_t sema) : sema_(sema) {
        if (__get_IPSR() != 0) {
            BaseType_t reschedule;
            xSemaphoreTakeFromISR(sema_, &reschedule);
            portYIELD_FROM_ISR(reschedule);
        } else {
            xSemaphoreTake(sema_, portMAX_DELAY);
        }
    }

    ~MutexLock() {
        if (__get_IPSR() != 0) {
            BaseType_t reschedule;
            xSemaphoreGiveFromISR(sema_, &reschedule);
            portYIELD_FROM_ISR(reschedule);
        } else {
            xSemaphoreGive(sema_);
        }
    }
    MutexLock(const MutexLock&) = delete;
    MutexLock(MutexLock&&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;
    MutexLock& operator=(MutexLock&&) = delete;

   private:
    SemaphoreHandle_t sema_;
};

class MulticoreMutexLock {
   public:
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
    MulticoreMutexLock(MutexLock&&) = delete;
    MulticoreMutexLock& operator=(const MutexLock&) = delete;
    MulticoreMutexLock& operator=(MutexLock&&) = delete;

   private:
    uint8_t gate_;
    uint8_t core_;
};

}  // namespace coral::micro

#endif  // _LIBS_BASE_MUTEX_H_