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

#ifndef Wire_h
#define Wire_h

#include <functional>

#include "Arduino.h"
#include "api/HardwareI2C.h"
#include "api/RingBuffer.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coralmicro {
namespace arduino {

typedef lpi2c_slave_transfer_t lpi2c_target_transfer_t;

// Allows for communication on the I2C bus.
// Example code can be found in `sketches/I2CM-Reader`, `sketches/I2CM-Writer`,
// `sketches/I2CS-Reader`, and `sketches/I2CS-Writer`, showing the
// different configurations for the communication.
// You should not initialize this object yourself; instead include `Wire.h` and
// then use the global `Wire` instance. Then, the desired configuration will
// determine how the communication works.
class HardwareI2C : public ::arduino::HardwareI2C {
 public:
  // @cond Do not generate docs.
  // Externally, use `begin()` and `end()`
  HardwareI2C(LPI2C_Type *);
  // @endcond

  // Joins the I2C bus as a controller.
  //
  void begin();

  // Joins the I2C bus as a peripheral.
  // @param address The peripheral address,
  //
  void begin(uint8_t address);

  // Leaves the I2C bus.
  //
  void end();

  // Sets the clock frequency for communication between devices.
  // @param freq The clock frequency.
  //
  void setClock(uint32_t freq);

  // Begins a transmission to the device at the given address.
  // @param address The address of the receiving device.
  //
  void beginTransmission(uint8_t address);

  // Sends the bytes queued with `write()`.
  // @param stopBit If stopBit is true, ends the transmission.
  // Otherwise, the transmission continues.
  //
  // @returns `kSuccess` if the transmission succeeds,
  // `kAddressNACK` if the peripheral device sends a NACK,
  // and `kOther` if there is a different error.
  uint8_t endTransmission(bool stopBit);

  // Ends the transmission, sending the bytes queued with `write()`.
  //
  // @returns `kSuccess` if the transmission succeeds,
  // `kAddressNACK` if the peripheral device sends a NACK,
  // and `kOther` if there is a different error.
  uint8_t endTransmission(void);

  // Allows the controller device to request data from the peripheral device.
  // @param address The address of the peripheral device
  // @param len The amount of data requested.
  // @param stopBit Stops the connection between the devices if true.
  // @returns The amount of data returned from the peripheral device.
  //
  size_t requestFrom(uint8_t address, size_t len, bool stopBit);

  // Allows the controller device to request data from the peripheral device.
  // @param address The address of the peripheral device
  // @param len The amount of data requested.
  // @returns The amount of data returned from the peripheral device.
  //
  size_t requestFrom(uint8_t address, size_t len);

  // Registers a callback called when the peripheral device receives
  // data from the controller device
  // @param cb The callback.
  //
  void onReceive(void (*)(int));

  // Registers a callback called when the controller devices requests
  // data from the peripheral device
  // @param cb The callback.
  //
  void onRequest(void (*)(void));

  // Writes data into the transmission.
  // @param c The data for the transmission.
  //
  size_t write(uint8_t c);

  // Writes data into the transmission.
  // @param str The data for the transmission.
  //
  size_t write(const char *str);

  // Writes data into the transmission.
  // @param buffer The data for the transmission.
  // @param size The size of the data in bytes.
  //
  size_t write(const uint8_t *buffer, size_t size);

  // The amount of data available when calling `read()`.
  // @returns The number of bytes available for reading.
  //
  int available();

  // Reads one byte from the transmission.
  // @returns The next byte in the transmission buffer.
  //
  int read();

  // Looks at the next byte in the transmission.
  // @returns The next byte in the transmission buffer,
  // without advancing.
  int peek();

  enum EndTransmissionStatus : uint8_t {
    kSuccess = 0,
    kDataTooLong = 1,
    kAddressNACK = 2,
    kDataNACK = 3,
    kOther = 4,
  };

 private:
  static void StaticTargetCallback(LPI2C_Type *base,
                                     lpi2c_target_transfer_t *transfer,
                                     void *userData);
  void TargetCallback(LPI2C_Type *base, lpi2c_target_transfer_t *transfer);
  static void StaticOnReceiveHandler(void *param);
  void OnReceiveHandler();
  LPI2C_Type *base_;
  lpi2c_rtos_handle_t handle_;
  lpi2c_slave_handle_t target_handle_;
  uint8_t tx_address_;
  std::function<void(int)> receive_cb_;
  std::function<void(void)> request_cb_;
  constexpr static size_t kBufferSize = 32;
  uint8_t tx_buffer_[kBufferSize];
  size_t tx_buffer_used_ = 0;
  ::arduino::RingBufferN<32> rx_buffer_;
  uint8_t isr_rx_buffer_[kBufferSize];
  uint8_t isr_tx_buffer_[kBufferSize];
  TaskHandle_t receive_task_handle_ = nullptr;

  constexpr static int kInterruptPriority = 3;
};

}  // namespace arduino
}  // namespace coralmicro

// This is the global `HardwareI2C` instance you should use instead of
// creating your own instance of `HardwareI2C`.
extern coralmicro::arduino::HardwareI2C Wire;

typedef coralmicro::arduino::HardwareI2C TwoWire;

#endif  // Wire_h
