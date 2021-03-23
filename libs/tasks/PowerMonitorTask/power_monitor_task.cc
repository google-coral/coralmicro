#include "libs/tasks/PowerMonitorTask/power_monitor_task.h"

#include <cmath>
#include <climits>

namespace valiant {

using namespace power_monitor;

constexpr const uint8_t kPowerMonitorAddress = 0x40;
constexpr const char kPowerMonitorTaskName[] = "kPowerMonitor_task";
constexpr float kMaximumExpectedCurrentAmps = 1;
constexpr int kTwoExpFifteen = 32768;
constexpr float kInternalScalingValue = 0.00512;
// TODO(atv): When we pick a real shunt value, update this.
constexpr int kShuntResistanceOhms = 2;
constexpr float kCurrentLsb = kMaximumExpectedCurrentAmps / kTwoExpFifteen;
constexpr float kVoltageM = 8.f;
constexpr float kVoltageR = 2.f;
// 1 / kCurrentLsb
constexpr float kCurrentM = 32768.f;
// No shift in kVoltageM
constexpr float kCurrentR = 0.f;
// 1 / (25 * kCurrentLsb)
constexpr float kPowerM = 13107.f;
// Decimal point of kPowerM was moved right by 1
constexpr float kPowerR = -1.0f;

void PowerMonitorTask::SendByte(Commands command) {
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPowerMonitorAddress;
    transfer.direction = kLPI2C_Write;
    transfer.subaddress = static_cast<uint8_t>(command);
    transfer.subaddressSize = sizeof(command);
    transfer.data = nullptr;
    transfer.dataSize = 0;
    LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PowerMonitorTask::WriteWord(Commands command, uint16_t word) {
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPowerMonitorAddress;
    transfer.direction = kLPI2C_Write;
    transfer.subaddress = static_cast<uint8_t>(command);
    transfer.subaddressSize = sizeof(command);
    transfer.data = &word;
    transfer.dataSize = sizeof(word);
    LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PowerMonitorTask::ReadWord(Commands command, uint16_t *word) {
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPowerMonitorAddress;
    transfer.direction = kLPI2C_Read;
    transfer.subaddress = static_cast<uint8_t>(command);
    transfer.subaddressSize = sizeof(command);
    transfer.data = word;
    transfer.dataSize = sizeof(*word);
    LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PowerMonitorTask::ReadByte(Commands command, uint8_t *byte) {
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPowerMonitorAddress;
    transfer.direction = kLPI2C_Read;
    transfer.subaddress = static_cast<uint8_t>(command);
    transfer.subaddressSize = sizeof(command);
    transfer.data = byte;
    transfer.dataSize = sizeof(*byte);
    LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PowerMonitorTask::ReadBlock(Commands command, uint8_t *block, size_t max_len) {
    uint8_t block_len = 0;
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferNoStopFlag;
    transfer.slaveAddress = kPowerMonitorAddress;
    transfer.direction = kLPI2C_Read;
    transfer.subaddress = static_cast<uint8_t>(command);
    transfer.subaddressSize = sizeof(command);
    transfer.data = &block_len;
    transfer.dataSize = sizeof(block_len);
    LPI2C_RTOS_Transfer(i2c_handle_, &transfer);

    if (block_len > max_len) {
        // TODO(atv): Return an error...
        return;
    }

    transfer.flags = kLPI2C_TransferNoStartFlag;
    transfer.data = block;
    transfer.dataSize = block_len;
    LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PowerMonitorTask::ConfigureDevice() {
    float cal = kInternalScalingValue / (kCurrentLsb * kShuntResistanceOhms);
    WriteWord(Commands::MFR_CALIBRATION, static_cast<uint16_t>(cal));

    constexpr int kAvgSamples = eAvgSamples16;
    constexpr int kAvgModeShift = 9;
    constexpr int kAvgModeMask = 0xE00;
    uint16_t adc_config;
    ReadWord(Commands::MFR_ADC_CONFIG, &adc_config);
    adc_config &= ~kAvgModeMask;
    adc_config |= (kAvgSamples << kAvgModeShift) & kAvgModeMask;
    WriteWord(Commands::MFR_ADC_CONFIG, adc_config);
}

void PowerMonitorTask::Init(lpi2c_rtos_handle_t* i2c_handle) {
    QueueTask::Init();
    i2c_handle_ = i2c_handle;
}

void PowerMonitorTask::TaskInit(void) {
    ConfigureDevice();
}

ChipIdResponse PowerMonitorTask::GetChipId() {
    Request req;
    req.type = RequestType::ChipId;
    Response resp = SendRequest(req);
    return resp.response.chip_id;
}

MeasurementResponse PowerMonitorTask::GetMeasurement() {
    Request req;
    req.type = RequestType::Measurement;
    Response resp = SendRequest(req);
    return resp.response.measurement;
}

ChipIdResponse PowerMonitorTask::HandleChipIdRequest() {
    ChipIdResponse resp;
    memset(&resp, 0, sizeof(resp));
    ReadBlock(Commands::MFR_ID, resp.mfr_id, sizeof(resp.mfr_id));
    ReadBlock(Commands::MFR_MODEL, resp.mfr_model, sizeof(resp.mfr_model));
    ReadBlock(Commands::MFR_REVISION, resp.mfr_revision, sizeof(resp.mfr_revision));
    ReadWord(Commands::TI_MFR_ID, reinterpret_cast<uint16_t*>(&resp.ti_mfr_id));
    ReadWord(Commands::TI_MFR_MODEL, reinterpret_cast<uint16_t*>(&resp.ti_mfr_model));
    ReadWord(Commands::TI_MFR_REVISION, reinterpret_cast<uint16_t*>(&resp.ti_mfr_revision));
    return resp;
}

MeasurementResponse PowerMonitorTask::HandleMeasurementRequest() {
    MeasurementResponse resp;
    memset(&resp, 0, sizeof(resp));
    uint16_t voltage_raw, kCurrentRaw, kPowerRaw;
    ReadWord(Commands::READ_VIN, &voltage_raw);
    ReadWord(Commands::READ_IIN, &kCurrentRaw);
    ReadWord(Commands::READ_PIN, &kPowerRaw);

    resp.voltage = 1 / kVoltageM * (static_cast<float>(voltage_raw) * std::pow(10.0f, -kVoltageR));
    resp.current = 1 / kCurrentM * (static_cast<float>(kCurrentRaw) * std::pow(10.0f, -kCurrentR));
    resp.power = 1 / kPowerM * (static_cast<float>(kPowerRaw) * std::pow(10.0f, -kPowerR));

    return resp;
}

void PowerMonitorTask::RequestHandler(Request *req) {
    Response resp;
    resp.type = req->type;
    switch (req->type) {
        case RequestType::ChipId:
            resp.response.chip_id = HandleChipIdRequest();
            break;
        case RequestType::Measurement:
            resp.response.measurement = HandleMeasurementRequest();
            break;
    }
    if (req->callback) {
        req->callback(resp);
    }
}

}  // namespace valiant
