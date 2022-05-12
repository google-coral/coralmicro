#ifndef _LIBS_TASKS_PMIC_TASK_H_
#define _LIBS_TASKS_PMIC_TASK_H_

#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coral::micro {

namespace pmic {

enum class RequestType : uint8_t {
    Rail,
    Gpio,
    ChipId,
};

enum class Rail : uint8_t {
    CAM_2V8,
    CAM_1V8,
    MIC_1V8,
};

struct RailRequest {
    Rail rail;
    bool enable;
};

enum class PmicGpio : uint8_t {
    DA_GPIO2,
    DA_GPIO3,
    DA_GPIO4,
};

enum class Direction : bool {
    In,
    Out,
};

struct GpioRequest {
    PmicGpio gpio;
    Direction dir;
    bool enable;
};

struct Response {
    RequestType type;
    union {
        uint8_t chip_id;
    } response;
};

struct Request {
    RequestType type;
    union {
        RailRequest rail;
        GpioRequest gpio;
    } request;
    std::function<void(Response)> callback;
};

enum class PmicRegisters : uint16_t {
    PAGE_CON = 0x000,
    LDO2_CONT = 0x027,
    LDO3_CONT = 0x028,
    LDO4_CONT = 0x029,
    DEVICE_ID = 0x181,
    UNKNOWN = 0xFFF,
};

}  // namespace pmic

static constexpr size_t kPmicTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kPmicTaskQueueLength = 4;
extern const char kPmicTaskName[];

class PmicTask : public QueueTask<pmic::Request, pmic::Response, kPmicTaskName,
                                  kPmicTaskStackDepth, PMIC_TASK_PRIORITY,
                                  kPmicTaskQueueLength> {
   public:
    void Init(lpi2c_rtos_handle_t* i2c_handle);
    static PmicTask* GetSingleton() {
        static PmicTask pmic;
        return &pmic;
    }
    void SetRailState(pmic::Rail rail, bool enable);
    uint8_t GetChipId();

   private:
    void TaskInit() override;
    void RequestHandler(pmic::Request* req) override;
    void HandleRailRequest(const pmic::RailRequest& rail);
    void HandleGpioRequest(const pmic::GpioRequest& gpio);
    uint8_t HandleChipIdRequest();
    void SetPage(uint16_t reg);
    void Read(pmic::PmicRegisters reg, uint8_t* val);
    void Write(pmic::PmicRegisters reg, uint8_t val);

    lpi2c_rtos_handle_t* i2c_handle_;
};

}  // namespace coral::micro

#endif  // _LIBS_TASKS_PMIC_TASK_H_
