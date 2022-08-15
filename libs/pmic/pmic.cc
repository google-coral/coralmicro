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

#include "libs/pmic/pmic.h"

#include <cstdio>

#include "libs/base/check.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coralmicro {
namespace {
constexpr uint8_t kPmicAddress = 0x58;
constexpr uint32_t kMaxTransferRetries = 10;

struct PmicRegisters {
  enum : uint16_t {
    kPageCon = 0x000,
    kLdo2Cont = 0x027,
    kLdo3Cont = 0x028,
    kLdo4Cont = 0x029,
    kDeviceId = 0x181,
    kUnknown = 0xFFF,
  };
};
}  // namespace

bool PmicTask::Read(uint16_t reg, uint8_t* val) {
  if (!SetPage(reg)) return false;

  lpi2c_master_transfer_t transfer;
  transfer.flags = kLPI2C_TransferDefaultFlag;
  transfer.slaveAddress = kPmicAddress;
  transfer.direction = kLPI2C_Read;
  transfer.subaddress = reg & 0xFF;
  transfer.subaddressSize = sizeof(uint8_t);
  transfer.data = val;
  transfer.dataSize = sizeof(*val);
  return Transfer(&transfer);
}

bool PmicTask::Write(uint16_t reg, uint8_t val) {
  if (!SetPage(reg)) return false;

  lpi2c_master_transfer_t transfer;
  transfer.flags = kLPI2C_TransferDefaultFlag;
  transfer.slaveAddress = kPmicAddress;
  transfer.direction = kLPI2C_Write;
  transfer.subaddress = reg & 0xFF;
  transfer.subaddressSize = sizeof(uint8_t);
  transfer.data = &val;
  transfer.dataSize = sizeof(val);
  return Transfer(&transfer);
}

bool PmicTask::SetPage(uint16_t reg) {
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
  return Transfer(&transfer);
}

bool PmicTask::Transfer(lpi2c_master_transfer_t* transfer) {
  status_t res = kStatus_Success;
  uint32_t attempts = 0;

  do {
    if (res == kStatus_LPI2C_Busy) {
      taskYIELD();
    } else if (res == kStatus_LPI2C_ArbitrationLost) {
      attempts++;
      if (attempts >= kMaxTransferRetries) {
        break;
      } else {
        // Retry right away.
      }
    }
    res = LPI2C_RTOS_Transfer(i2c_handle_, transfer);
  } while ((res == kStatus_LPI2C_Busy) ||
           (res == kStatus_LPI2C_ArbitrationLost));

  return res == kStatus_Success;
}

void PmicTask::Init(lpi2c_rtos_handle_t* i2c_handle) {
  QueueTask::Init();
  i2c_handle_ = i2c_handle;
}

void PmicTask::HandleRailRequest(const pmic::RailRequest& rail) {
  auto reg = PmicRegisters::kUnknown;
  uint8_t val;
  switch (rail.rail) {
    case PmicRail::kCam2V8:
      reg = PmicRegisters::kLdo2Cont;
      break;
    case PmicRail::kCam1V8:
      reg = PmicRegisters::kLdo3Cont;
      break;
    case PmicRail::kMic1V8:
      reg = PmicRegisters::kLdo4Cont;
      break;
  }
  CHECK(Read(reg, &val));
  if (rail.enable) {
    val |= 1;
  } else {
    val &= ~1;
  }
  CHECK(Write(reg, val));
}

uint8_t PmicTask::HandleChipIdRequest() {
  uint8_t device_id = 0xff;
  CHECK(Read(PmicRegisters::kDeviceId, &device_id));
  return device_id;
}

void PmicTask::RequestHandler(pmic::Request* req) {
  pmic::Response resp;
  resp.type = req->type;
  switch (req->type) {
    case pmic::RequestType::kRail:
      HandleRailRequest(req->request.rail);
      break;
    case pmic::RequestType::kChipId:
      resp.response.chip_id = HandleChipIdRequest();
      break;
  }
  if (req->callback) req->callback(resp);
}

void PmicTask::SetRailState(PmicRail rail, bool enable) {
  pmic::Request req;
  req.type = pmic::RequestType::kRail;
  req.request.rail.rail = rail;
  req.request.rail.enable = enable;
  SendRequest(req);
}

uint8_t PmicTask::GetChipId() {
  pmic::Request req;
  req.type = pmic::RequestType::kChipId;
  pmic::Response resp = SendRequest(req);
  return resp.response.chip_id;
}

}  // namespace coralmicro
