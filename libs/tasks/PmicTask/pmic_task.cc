#include "libs/tasks/PmicTask/pmic_task.h"

#include <cstdio>

#include "libs/base/check.h"
#include "libs/base/mutex.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coral::micro {
namespace {
constexpr uint8_t kPmicAddress = 0x58;

struct PmicRegisters {
    enum : uint16_t {
        PAGE_CON = 0x000,
        LDO2_CONT = 0x027,
        LDO3_CONT = 0x028,
        LDO4_CONT = 0x029,
        DEVICE_ID = 0x181,
        UNKNOWN = 0xFFF,
    };
};
}  // namespace

void PmicTask::Read(uint16_t reg, uint8_t* val) {
    uint8_t offset = reg & 0xFF;

    SetPage(reg);
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPmicAddress;
    transfer.direction = kLPI2C_Read;
    transfer.subaddress = offset;
    transfer.subaddressSize = sizeof(uint8_t);
    transfer.data = val;
    transfer.dataSize = sizeof(*val);
    CHECK(LPI2C_RTOS_Transfer(i2c_handle_, &transfer) == kStatus_Success);
}

void PmicTask::Write(uint16_t reg, uint8_t val) {
    uint8_t offset = reg & 0xFF;

    SetPage(reg);
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPmicAddress;
    transfer.direction = kLPI2C_Write;
    transfer.subaddress = offset;
    transfer.subaddressSize = sizeof(uint8_t);
    transfer.data = &val;
    transfer.dataSize = sizeof(val);
    CHECK(LPI2C_RTOS_Transfer(i2c_handle_, &transfer) == kStatus_Success);
}

void PmicTask::SetPage(uint16_t reg) {
    uint8_t page = (reg >> 7) & 0x3;
    // Revert after transaction (probably not ideal. cache our page and only
    // change as needed)
    uint8_t page_con_reg = 0x80 | page;

    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPmicAddress;
    transfer.direction = kLPI2C_Write;
    transfer.subaddress = static_cast<uint32_t>(PmicRegisters::PAGE_CON);
    transfer.subaddressSize = sizeof(uint8_t);
    transfer.data = &page_con_reg;
    transfer.dataSize = sizeof(page_con_reg);
    CHECK(LPI2C_RTOS_Transfer(i2c_handle_, &transfer) == kStatus_Success);
}

PmicTask::PmicTask(): mutex_(xSemaphoreCreateMutex()) {
    CHECK(mutex_);
}

PmicTask::~PmicTask() { vSemaphoreDelete(mutex_); }

void PmicTask::SetRailState(PmicRail rail, bool enable) {
    MutexLock lock(mutex_);

    uint16_t reg = PmicRegisters::UNKNOWN;
    uint8_t val;
    switch (rail) {
        case PmicRail::kCam2V8:
            reg = PmicRegisters::LDO2_CONT;
            break;
        case PmicRail::kCam1V8:
            reg = PmicRegisters::LDO3_CONT;
            break;
        case PmicRail::kMic1V8:
            reg = PmicRegisters::LDO4_CONT;
            break;
    }
    Read(reg, &val);
    if (enable) {
        val |= 1;
    } else {
        val &= ~1;
    }
    Write(reg, val);
}

uint8_t PmicTask::GetChipId() {
    MutexLock lock(mutex_);

    uint8_t device_id = 0xff;
    Read(PmicRegisters::DEVICE_ID, &device_id);
    return device_id;
}

}  // namespace coral::micro
