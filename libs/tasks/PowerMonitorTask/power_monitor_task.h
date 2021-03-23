#ifndef _LIBS_TASKS_POWER_MONITOR_TASK_POWER_MONITOR_TASK_H_
#define _LIBS_TASKS_POWER_MONITOR_TASK_POWER_MONITOR_TASK_H_

#include "libs/base/queue_task.h"
#include "libs/base/tasks_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include <functional>

namespace valiant {

namespace power_monitor {

enum class Commands : uint8_t {
    CLEAR_FAULTS = 0x03,
    RESTORE_DEFAULT_ALL = 0x12,
    CAPABILITY = 0x19,
    IOUT_OC_WARN_LIMIT = 0x4A,
    VIN_OV_WARN_LIMIT = 0x57,
    VIN_UV_WARN_LIMIT = 0x58,
    PIN_OP_WARN_LIMIT = 0x6B,
    STATUS_BYTE = 0x78,
    STATUS_WORD = 0x79,
    STATUS_IOUT = 0x7B,
    STATUS_INPUT = 0x7C,
    STATUS_CML = 0x7E,
    STATUS_MFR_SPECIFIC = 0x80,
    READ_EIN = 0x86,
    READ_VIN = 0x88,
    READ_IIN = 0x89,
    READ_VOUT = 0x8B,
    READ_IOUT = 0x8C,
    READ_POUT = 0x96,
    READ_PIN = 0x97,
    MFR_ID = 0x99,
    MFR_MODEL = 0x9A,
    MFR_REVISION = 0x9B,
    MFR_ADC_CONFIG = 0xD0,
    MFR_READ_VSHUNT = 0xD1,
    MFR_ALERT_MASK = 0xD2,
    MFR_CALIBRATION = 0xD4,
    MFR_DEVICE_CONFIG = 0xD5,
    CLEAR_EIN = 0xD6,
    TI_MFR_ID = 0xE0,
    TI_MFR_MODEL = 0xE1,
    TI_MFR_REVISION = 0xE2,
};

enum class RequestType : uint8_t {
    ChipId,
    Measurement,
};

constexpr uint16_t kMfrId = 0x5449;
constexpr uint16_t kMfrModel = 0x3333;
constexpr uint16_t kMfrRevision = 0x4130;
constexpr const char kMfrModelStr[] = "INA233";
struct ChipIdResponse {
    /* PMBus block read */
    uint8_t mfr_id[2];
    uint8_t mfr_model[6];
    uint8_t mfr_revision[2];
    /* Standard I2C reads */
    uint16_t ti_mfr_id;
    uint16_t ti_mfr_model;
    uint16_t ti_mfr_revision;
};

struct MeasurementResponse {
    float voltage;
    float current;
    float power;
};

struct Response {
    RequestType type;
    union {
        ChipIdResponse chip_id;
        MeasurementResponse measurement;
    } response;
};

struct Request {
    RequestType type;
    union {
    } request;
    std::function<void(Response)> callback;
};

}  // namespace power_monitor

static constexpr size_t kPowerMonitorTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kPowerMonitorTaskQueueLength = 4;
extern const char kPowerMonitorTaskName[];

class PowerMonitorTask : public QueueTask<power_monitor::Request, power_monitor::Response, kPowerMonitorTaskName,
                                          kPowerMonitorTaskStackDepth, POWER_MONITOR_TASK_PRIORITY, kPowerMonitorTaskQueueLength> {
  public:
    void Init(lpi2c_rtos_handle_t *i2c_handle);
    static PowerMonitorTask *GetSingleton() {
        static PowerMonitorTask power_monitor;
        return &power_monitor;
    }

    power_monitor::ChipIdResponse GetChipId();
    power_monitor::MeasurementResponse GetMeasurement();
  private:
    enum {
        eAvgSamples1 = 0,
        eAvgSamples4 = 1,
        eAvgSamples16 = 2,
        eAvgSamples64 = 3,
        eAvgSamples128 = 4,
        eAvgSamples256 = 5,
        eAvgSamples512 = 6,
        eAvgSamples1024 = 7,
    };

    void TaskInit() override;
    void RequestHandler(power_monitor::Request *req) override;
    power_monitor::ChipIdResponse HandleChipIdRequest();
    power_monitor::MeasurementResponse HandleMeasurementRequest();

    void ConfigureDevice();

    void SendByte(power_monitor::Commands command);
    void WriteWord(power_monitor::Commands command, uint16_t word);
    void ReadWord(power_monitor::Commands command, uint16_t *word);
    void ReadByte(power_monitor::Commands command, uint8_t *byte);
    void ReadBlock(power_monitor::Commands command, uint8_t *block, size_t max_len);

    lpi2c_rtos_handle_t* i2c_handle_;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_POWER_MONITOR_TASK_POWER_MONITOR_TASK_H_
