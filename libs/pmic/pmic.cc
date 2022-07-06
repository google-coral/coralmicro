// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "libs/pmic/pmic.h"

#include <cstdio>

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coralmicro {
using namespace pmic;
namespace {
constexpr uint8_t kPmicAddress = 0x58;
}  // namespace

void PmicTask::Read(PmicRegisters reg, uint8_t* val) {
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
    // Revert after transaction (probably not ideal. cache our page and only
    // change as needed)
    uint8_t page_con_reg = 0x80 | page;

    lpi2c_master_transfer_t transfer;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = kPmicAddress;
    transfer.direction = kLPI2C_Write;
    transfer.subaddress = static_cast<uint32_t>(PmicRegisters::kPageCon);
    transfer.subaddressSize = sizeof(uint8_t);
    transfer.data = &page_con_reg;
    transfer.dataSize = sizeof(page_con_reg);
    /* status_t status = */ LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
}

void PmicTask::Init(lpi2c_rtos_handle_t* i2c_handle) {
    QueueTask::Init();
    i2c_handle_ = i2c_handle;
}

void PmicTask::HandleRailRequest(const RailRequest& rail) {
    PmicRegisters reg = PmicRegisters::kUnknown;
    uint8_t val;
    switch (rail.rail) {
        case Rail::kCam2V8:
            reg = PmicRegisters::kLdo2Cont;
            break;
        case Rail::kCam1V8:
            reg = PmicRegisters::kLdo3Cont;
            break;
        case Rail::kMic1V8:
            reg = PmicRegisters::kLdo4Cont;
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

uint8_t PmicTask::HandleChipIdRequest() {
    uint8_t device_id = 0xff;
    Read(PmicRegisters::kDeviceId, &device_id);
    return device_id;
}

void PmicTask::RequestHandler(Request* req) {
    Response resp;
    resp.type = req->type;
    switch (req->type) {
        case RequestType::kRail:
            HandleRailRequest(req->request.rail);
            break;
        case RequestType::kChipId:
            resp.response.chip_id = HandleChipIdRequest();
            break;
    }
    if (req->callback) req->callback(resp);
}

void PmicTask::SetRailState(Rail rail, bool enable) {
    Request req;
    req.type = RequestType::kRail;
    req.request.rail.rail = rail;
    req.request.rail.enable = enable;
    SendRequest(req);
}

uint8_t PmicTask::GetChipId() {
    Request req;
    req.type = RequestType::kChipId;
    Response resp = SendRequest(req);
    return resp.response.chip_id;
}

}  // namespace coralmicro
