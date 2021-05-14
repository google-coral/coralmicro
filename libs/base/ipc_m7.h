#ifndef __LIBS_BASE_IPC_M7_H__
#define __LIBS_BASE_IPC_M7_H__

#include "libs/base/ipc.h"
#include <cstdint>
#include <functional>


namespace valiant {
class IPCM7 : public IPC {
  public:
    void Init() override;
    void StartM4();
    bool M4IsAlive(uint32_t millis);
    static bool HasM4Application();
  protected:
    void TxTaskFn(void *param) override;
  private:
    static void StaticRemoteAppEventHandler(uint16_t eventData, void *context);
    void RemoteAppEventHandler(uint16_t eventData, void *context);
    void HandleSystemMessage(const ipc::SystemMessage& message) override;

    static constexpr size_t kMessageBufferSize = (8 * sizeof(ipc::Message));
    static uint8_t tx_queue_storage_[kMessageBufferSize + sizeof(ipc::MessageBuffer)] __attribute__((section(".noinit.$rpmsg_sh_mem")));
    static uint8_t rx_queue_storage_[kMessageBufferSize + sizeof(ipc::MessageBuffer)] __attribute__((section(".noinit.$rpmsg_sh_mem")));
};
}  // namespace valiant

#endif  // __LIBS_BASE_IPC_M7_H__
