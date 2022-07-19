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

#ifndef LIBS_TPU_EDGETPU_DRIVER_H_
#define LIBS_TPU_EDGETPU_DRIVER_H_

#include <cstdint>
#include <vector>

#include "libs/tpu/darwinn/driver/config/beagle/beagle_chip_config.h"
#include "libs/tpu/darwinn/driver/hardware_structures.h"
#include "libs/tpu/usb_host_edgetpu.h"

namespace coralmicro {

enum class PerformanceMode {
  kLow,
  kMedium,
  kHigh,
  kMax,
};

enum class DescriptorTag {
  kUnknown = -1,
  kInstructions = 0,
  kInputActivations = 1,
  kParameters = 2,
  kOutputActivations = 3,
  kInterrupt0 = 4,
  kInterrupt1 = 5,
  kInterrupt2 = 6,
  kInterrupt3 = 7,
};

class TpuDriver {
 public:
  TpuDriver() = default;
  TpuDriver(const TpuDriver&) = delete;
  TpuDriver& operator=(const TpuDriver&) = delete;
  bool Initialize(usb_host_edgetpu_instance_t* usb_instance,
                  PerformanceMode mode);
  bool SendParameters(const uint8_t* data, uint32_t length) const;
  bool SendInputs(const uint8_t* data, uint32_t length) const;
  bool SendInstructions(const uint8_t* data, uint32_t length) const;
  bool GetOutputs(uint8_t* data, uint32_t length) const;
  bool ReadEvent() const;
  float GetTemperature();

 private:
  enum class RegisterSize {
    kRegSize32,
    kRegSize64,
  };

  bool BulkOutTransfer(const uint8_t* data, uint32_t data_length) const;
  ssize_t BulkOutTransferInternal(uint8_t endpoint, const uint8_t* data,
                                  uint32_t data_length) const;
  bool BulkInTransfer(uint8_t* data, uint32_t data_length) const;
  ssize_t BulkInTransferInternal(uint8_t endpoint, uint8_t* data,
                                 uint32_t data_length) const;

  bool SendData(DescriptorTag tag, const uint8_t* data, uint32_t length) const;
  bool WriteHeader(DescriptorTag tag, uint32_t length) const;
  std::vector<uint8_t> PrepareHeader(DescriptorTag tag, uint32_t length) const;

  bool CSRTransfer(uint64_t reg, void* data, bool read, RegisterSize reg_size);
  bool Read32(uint64_t reg, uint32_t* val);
  bool Read64(uint64_t reg, uint64_t* val);
  bool Write32(uint64_t reg, uint32_t val);
  bool Write64(uint64_t reg, uint64_t val);
  bool DoRunControl(platforms::darwinn::driver::RunControl run_state);

  platforms::darwinn::driver::config::BeagleChipConfig chip_config_;
  usb_host_edgetpu_instance_t* usb_instance_ = nullptr;
};

}  // namespace coralmicro

#endif  // LIBS_TPU_EDGETPU_DRIVER_H_
