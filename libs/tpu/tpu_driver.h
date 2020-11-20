#ifndef _LIBS_TPU_TPU_DRIVER_H_
#define _LIBS_TPU_TPU_DRIVER_H_

#include "libs/usb_host_edgetpu/usb_host_edgetpu.h"
#include "third_party/darwinn/driver/hardware_structures.h"
#include "third_party/darwinn/driver/config/beagle/beagle_chip_config.h"

#include <cstdint>
#include <vector>

namespace valiant {

static const uint8_t kSingleBulkOutEndpoint = 1;
static const uint8_t kEventInEndpoint = 2;
static const uint8_t kInterruptInEndpoint = 3;

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
    TpuDriver();
    TpuDriver(const TpuDriver&) = delete;
    TpuDriver& operator=(const TpuDriver&) = delete;
    bool Initialize(usb_host_edgetpu_instance_t *usb_instance);
    bool SendParameters(const uint8_t *data, uint32_t length) const;
    bool SendInputs(const uint8_t *data, uint32_t length) const;
    bool SendInstructions(const uint8_t *data, uint32_t length) const;
    bool GetOutputs(uint8_t *data, uint32_t length) const;
    bool ReadEvent() const;

  private:
    enum class RegisterSize {
      kRegSize32,
      kRegSize64,
    };

    bool BulkOutTransfer(const uint8_t *data, uint32_t data_length) const;
    bool BulkOutTransferInternal(uint8_t endpoint, const uint8_t *data, uint32_t data_length) const;
    bool BulkInTransfer(uint8_t *data, uint32_t data_length) const;
    bool BulkInTransferInternal(uint8_t endpoint, uint8_t *data, uint32_t data_length) const;

    bool SendData(DescriptorTag tag, const uint8_t *data, uint32_t length) const;
    bool WriteHeader(DescriptorTag tag, uint32_t length) const;
    std::vector<uint8_t> PrepareHeader(DescriptorTag tag, uint32_t length) const;

    bool CSRTransfer(uint64_t reg, void *data, bool read, RegisterSize reg_size);
    bool Read32(uint64_t reg, uint32_t* val);
    bool Read64(uint64_t reg, uint64_t* val);
    bool Write32(uint64_t reg, uint32_t val);
    bool Write64(uint64_t reg, uint64_t val);
    bool DoRunControl(platforms::darwinn::driver::RunControl run_state);

    platforms::darwinn::driver::config::BeagleChipConfig chip_config_;
    usb_host_edgetpu_instance_t* usb_instance_ = nullptr;
};

}  // namespace valiant

#endif  // _LIBS_TPU_TPU_DRIVER_H_
