#ifndef __LIBS_CONSOLE_CONSOLE_M7_H__
#define __LIBS_CONSOLE_CONSOLE_M7_H__

#include "libs/CdcAcm/cdc_acm.h"
#include "libs/base/message_buffer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"

namespace valiant {

class ConsoleM7 {
  public:
    static ConsoleM7* GetSingleton() {
        static ConsoleM7 console;
        return &console;
    }
    void Init();
    StreamBuffer* GetM4ConsoleBufferPtr();
    void Write(char *buffer, int size);
  private:
    struct ConsoleMessage {
        int len;
        uint8_t *str;
    };

    static void StaticM4ConsoleTaskFn(void *param);
    void M4ConsoleTaskFn(void *param);
    static void StaticM7ConsoleTaskFn(void *param);
    void M7ConsoleTaskFn(void *param);

    ConsoleM7() {}
    ConsoleM7(const ConsoleM7&) = delete;
    ConsoleM7& operator=(const ConsoleM7&) = delete;

    QueueHandle_t console_queue_ = nullptr;
    CdcAcm cdc_acm_;

    StreamBuffer *m4_console_buffer_ = nullptr;
    static constexpr size_t kM4ConsoleBufferBytes = 128;
    static constexpr size_t kM4ConsoleBufferSize = kM4ConsoleBufferBytes + sizeof(StreamBuffer);
    static uint8_t m4_console_buffer_storage_[kM4ConsoleBufferSize];
};

}  // namespace valiant

#endif  // __LIBS_CONSOLE_CONSOLE_M7_H__
