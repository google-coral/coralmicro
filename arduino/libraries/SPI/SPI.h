#ifndef SPI_H_
#define SPI_H_

#include "Arduino.h"
#include "api/Common.h"
#include "api/HardwareSPI.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi_freertos.h"

namespace valiant {
namespace arduino {

class HardwareSPI : public ::arduino::HardwareSPI {
 public:
  HardwareSPI(LPSPI_Type*);

  uint8_t transfer(uint8_t data);
  uint16_t transfer16(uint16_t data);
  void transfer(void* buf, size_t count);

  // Transaction Functions
  void beginTransaction(::arduino::SPISettings settings);
  void endTransaction(void);

  // attachInterrupt and detachInterrupt are undocumented and should not be
  // used, so they are unimplemented here.
  void attachInterrupt();
  void detachInterrupt();
  void usingInterrupt(int interruptNumber);
  void notUsingInterrupt(int interruptNumber);

  void begin();
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
}  // namespace valiant

extern valiant::arduino::HardwareSPI SPI;

#endif  // SPI_H_