#ifndef _LIBS_TASKS_PMIC_TASK_H_
#define _LIBS_TASKS_PMIC_TASK_H_

#include <cstdint>

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coral::micro {

enum class PmicRail : uint8_t {
    kCam2V8,
    kCam1V8,
    kMic1V8,
};

class PmicTask {
   public:
    PmicTask();
    ~PmicTask();

    void Init(lpi2c_rtos_handle_t* i2c_handle) { i2c_handle_ = i2c_handle; }
    static PmicTask* GetSingleton() {
        static PmicTask pmic;
        return &pmic;
    }
    void SetRailState(PmicRail rail, bool enable);
    uint8_t GetChipId();

   private:
    void SetPage(uint16_t reg);
    void Read(uint16_t reg, uint8_t* val);
    void Write(uint16_t reg, uint8_t val);

    SemaphoreHandle_t mutex_;
    lpi2c_rtos_handle_t* i2c_handle_;
};

}  // namespace coral::micro

#endif  // _LIBS_TASKS_PMIC_TASK_H_
