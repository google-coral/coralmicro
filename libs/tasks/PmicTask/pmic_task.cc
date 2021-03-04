#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#include <cstdio>

namespace valiant {

constexpr const uint8_t kPmicAddress = 0x58;
constexpr const char kPmicTaskName[] = "pmic_task";

void PmicTask::Read(PmicRegisters reg, uint8_t *val) {
    uint8_t offset = (static_cast<uint16_t>(reg) & 0xFF);

    SetPage(static_cast<uint16_t>(reg));
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPmicAddress;
    transfer.direction = kLPI2C_Read;
    transfer.subaddress = offset;
    transfer.subaddressSize = sizeof(uint8_t);
    transfer.data = val;
    transfer.dataSize = sizeof(*val);
    /* status_t status = */ LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PmicTask::Write(PmicRegisters reg, uint8_t val) {
    uint8_t offset = (static_cast<uint16_t>(reg) & 0xFF);

    SetPage(static_cast<uint16_t>(reg));
    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPmicAddress;
    transfer.direction = kLPI2C_Write;
    transfer.subaddress = offset;
    transfer.subaddressSize = sizeof(uint8_t);
    transfer.data = &val;
    transfer.dataSize = sizeof(val);
    /* status_t status = */ LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PmicTask::SetPage(uint16_t reg) {
    uint8_t page = (reg >> 7) & 0x3;
    uint8_t page_con_reg = 0x80 | page; // Revert after transaction (probably not ideal. cache our page and only change as needed)

    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPmicAddress;
    transfer.direction = kLPI2C_Write;
    transfer.subaddress = static_cast<uint32_t>(PmicRegisters::PAGE_CON);
    transfer.subaddressSize = sizeof(uint8_t);
    transfer.data = &page_con_reg;
    transfer.dataSize = sizeof(page_con_reg);
    /* status_t status = */ LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PmicTask::Init(lpi2c_rtos_handle_t *i2c_handle) {
    QueueTask::Init();
    i2c_handle_ = i2c_handle;
}

void PmicTask::TaskInit() {
    uint8_t device_id = 0xff;
    Read(PmicRegisters::DEVICE_ID, &device_id);
    printf("PMIC Device ID: 0x%x\r\n", device_id);
}

void PmicTask::HandleRailRequest(const RailRequest& rail) {
    PmicRegisters reg = PmicRegisters::UNKNOWN;
    uint8_t val;
    switch (rail.rail) {
        case Rail::CAM_2V8:
            reg = PmicRegisters::LDO2_CONT;
            break;
        case Rail::CAM_1V8:
            reg = PmicRegisters::LDO3_CONT;
            break;
        case Rail::MIC_1V8:
            reg = PmicRegisters::LDO4_CONT;
            break;
    }
    Read(reg, &val);
    if (rail.enable) {
        val |= 1;
    } else {
        val &= ~1;
    }
    Write(reg, val);
}

void PmicTask::HandleGpioRequest(const GpioRequest& gpio) {
}

void PmicTask::MessageHandler(PmicRequest *req) {
    switch (req->type) {
        case PmicRequestType::Rail:
            HandleRailRequest(req->request.rail);
            break;
        case PmicRequestType::Gpio:
            HandleGpioRequest(req->request.gpio);
            break;
    }
    if (req->callback)
        req->callback();
}

void PmicTask::SetRailState(Rail rail, bool enable) {
    SemaphoreHandle_t req_semaphore = xSemaphoreCreateBinary();
    PmicRequest req;
    req.type = PmicRequestType::Rail;
    req.request.rail.rail = rail;
    req.request.rail.enable = enable;
    req.callback =
        [req_semaphore]() {
            xSemaphoreGive(req_semaphore);
        };
    xQueueSend(message_queue_, &req, pdMS_TO_TICKS(200));
    xSemaphoreTake(req_semaphore, pdMS_TO_TICKS(200));
    vSemaphoreDelete(req_semaphore);
}

}  // namespace valiant
