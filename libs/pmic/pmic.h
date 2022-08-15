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

#ifndef LIBS_PMIC_PMIC_H_
#define LIBS_PMIC_PMIC_H_

#include <cstdint>
#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coralmicro {

enum class PmicRail : uint8_t {
  kCam2V8,
  kCam1V8,
  kMic1V8,
};

namespace pmic {

enum class RequestType : uint8_t {
  kRail,
  kChipId,
};

struct RailRequest {
  PmicRail rail;
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
  } request;
  std::function<void(Response)> callback;
};

}  // namespace pmic

inline constexpr char kPmicTaskName[] = "pmic_task";

class PmicTask : public QueueTask<pmic::Request, pmic::Response, kPmicTaskName,
                                  configMINIMAL_STACK_SIZE * 10,
                                  kPmicTaskPriority, /*QueueLength=*/4> {
 public:
  void Init(lpi2c_rtos_handle_t* i2c_handle);
  static PmicTask* GetSingleton() {
    static PmicTask pmic;
    return &pmic;
  }
  void SetRailState(PmicRail rail, bool enable);
  uint8_t GetChipId();

 private:
  void RequestHandler(pmic::Request* req) override;
  void HandleRailRequest(const pmic::RailRequest& rail);
  uint8_t HandleChipIdRequest();
  bool SetPage(uint16_t reg);
  bool Read(uint16_t reg, uint8_t* val);
  bool Write(uint16_t reg, uint8_t val);
  bool Transfer(lpi2c_master_transfer_t* transfer);

  lpi2c_rtos_handle_t* i2c_handle_;
};

}  // namespace coralmicro

#endif  // LIBS_PMIC_PMIC_H_
