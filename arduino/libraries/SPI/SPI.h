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

#ifndef SPI_H_
#define SPI_H_

#include "Arduino.h"
#include "api/Common.h"
#include "api/HardwareSPI.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi_freertos.h"

namespace coralmicro {
namespace arduino {

// Allows for communication with SPI devices.
// SPI devices communicate through the use of the `transfer` functions,
// which simultaneously exchange data between the two devices, but they
// cannot be used from within interrupts.
//
// You should not initialize this object yourself;
// instead include `SPI.h` and then use the global `SPI` instance. Code samples
// can be found in `sketches/SPI/` and `sketches/SPITranscation/`.
class HardwareSPI : public ::arduino::HardwareSPI {
 public:
  // @cond Do not generate docs.
  // Externally, use `begin()` and `end()`
  HardwareSPI(LPSPI_Type*);
  // @endcond

  // Exchanges one byte of data with SPI transfer.
  //
  // @param data The data for the peripheral device.
  // @returns The data received from the peripheral device.
  uint8_t transfer(uint8_t data);

  // Exchanges two bytes of data with SPI transfer.
  //
  // @param data The data for the peripheral device.
  // @returns The data received from the peripheral device.
  uint16_t transfer16(uint16_t data);

  // Exchanges an array of data in-place with SPI transfer.
  //
  // @param buf The data for the peripheral device.  As
  // data is received, the buffer is overwritten.
  // @param count The length of the data in bytes.
  void transfer(void* buf, size_t count);

  // Updates the SPI configuration.
  //
  // @param settings The desired SPI configuration.
  //   See [SPISettings](https://www.arduino.cc/reference/en/language/functions/communication/spi/spisettings/).
  void updateSettings(::arduino::SPISettings settings);

  // @cond Do not generate docs.
  // Our SPI library does not support use with interrupts.
  void beginTransaction(::arduino::SPISettings settings);
  void endTransaction(void);
  // @endcond

  // @cond Do not generate docs.
  // attachInterrupt and detachInterrupt are undocumented and should not be
  // used, so they are unimplemented here.
  void attachInterrupt();
  void detachInterrupt();
  void usingInterrupt(int interruptNumber);
  void notUsingInterrupt(int interruptNumber);
  // @endcond

  // Initializes SPI.
  //
  // This function must be called before doing any transfers.
  void begin();

  // De-initializes SPI.
  //
  void end();

 private:
  uint32_t GetConfigFlags();

  LPSPI_Type* base_;
  lpspi_rtos_handle_t handle_;
  lpspi_master_config_t config_;

  constexpr static int kInterruptPriority = 3;

  bool initialized_ = false;
};

}  // namespace arduino
}  // namespace coralmicro

// This is the global `HardwareSPI` instance you should use instead of
// creating your own instance.
extern coralmicro::arduino::HardwareSPI SPI;

#endif  // SPI_H_
