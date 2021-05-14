#ifndef _LIBS_BASE_IPC_H_
#define _LIBS_BASE_IPC_H_

#include "libs/base/message_buffer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"

#include <functional>

namespace valiant {

class IPC {
  public:
    using AppMessageHandler = std::function<void(const uint8_t data[ipc::kMessageBufferDataSize], void*)>;
    static IPC* GetSingleton();
    virtual void Init();
    void SendMessage(const ipc::Message& message);
    void RegisterAppMessageHandler(AppMessageHandler, void *param);
  private:
    static void StaticFreeRtosMessageEventHandler(uint16_t eventData, void *context);
    void FreeRtosMessageEventHandler(uint16_t eventData, void *context);
    static void StaticTxTaskFn(void *param);
    static void StaticRxTaskFn(void *param);
    AppMessageHandler app_handler_ = nullptr;
    constexpr static int kSendMessageNotification = 1;
    void* app_handler_param_ = nullptr;
  protected:
    void HandleAppMessage(const uint8_t data[ipc::kMessageBufferDataSize]);
    virtual void HandleSystemMessage(const ipc::SystemMessage& message) = 0;
    virtual void TxTaskFn(void *param);
    virtual void RxTaskFn(void *param);
    SemaphoreHandle_t tx_semaphore_;
    TaskHandle_t tx_task_, rx_task_;
    ipc::MessageBuffer *tx_queue_, *rx_queue_;
};

}  // namespace valiant

#endif  // _LIBS_BASE_IPC_H_