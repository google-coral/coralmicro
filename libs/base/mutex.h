#ifndef _LIBS_BASE_MUTEX_H_
#define _LIBS_BASE_MUTEX_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"

namespace valiant {
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
}

#endif  // _LIBS_BASE_MUTEX_H_