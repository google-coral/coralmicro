#ifndef Wire_h
#define Wire_h

#include "Arduino.h"
#include "api/HardwareI2C.h"
#include "api/RingBuffer.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#include <functional>

namespace valiant {
namespace arduino {

class HardwareI2C : public ::arduino::HardwareI2C {
  public:
    HardwareI2C(LPI2C_Type*);
    void begin();
    void begin(uint8_t address);
    void end();

    void setClock(uint32_t freq);
  
    void beginTransmission(uint8_t address);
    uint8_t endTransmission(bool stopBit);
    uint8_t endTransmission(void);

    size_t requestFrom(uint8_t address, size_t len, bool stopBit) ;
    size_t requestFrom(uint8_t address, size_t len);

    void onReceive(void(*)(int));
    void onRequest(void(*)(void));

    size_t write(uint8_t c);
    size_t write(const char *str);
    size_t write(const uint8_t *buffer, size_t size);
    int available();
    int read();
    int peek();

    enum EndTransmissionStatus : uint8_t {
        kSuccess = 0,
        kDataTooLong = 1,
        kAddressNACK = 2,
        kDataNACK = 3,
        kOther = 4,
    };

  private:
    static void StaticSlaveCallback(LPI2C_Type *base, lpi2c_slave_transfer_t *transfer, void *userData);
    void SlaveCallback(LPI2C_Type *base, lpi2c_slave_transfer_t *transfer);
    static void StaticOnReceiveHandler(void *param);
    void OnReceiveHandler();
    LPI2C_Type* base_;
    lpi2c_rtos_handle_t handle_;
    lpi2c_slave_handle_t slave_handle_;
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
}  // namespace valiant

extern valiant::arduino::HardwareI2C Wire;

#endif  // Wire_h
