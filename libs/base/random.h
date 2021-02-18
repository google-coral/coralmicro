#ifndef _LIBS_BASE_RANDOM_H_
#define _LIBS_BASE_RANDOM_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/queue.h"
#include "third_party/freertos_kernel/include/task.h"
#include <cstddef>
#include <functional>

namespace valiant {

class Random {
  public:
    static Random *GetSingleton() {
        static Random random;
        return &random;
    }
    void Init();
    bool GetRandomNumber(void *out, size_t len);
  private:
    struct RandomRequest {
        void *out;
        size_t len;
        std::function<void(bool)> callback;
    };
    static void StaticTaskMain(void *param);
    void TaskMain(void *param);

    static constexpr size_t kRandomTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
    StaticTask_t random_task_;
    StackType_t random_task_stack_[kRandomTaskStackDepth];
    QueueHandle_t request_queue_;
};

}  // namespace valiant

#endif  // _LIBS_BASE_RANDOM_H_
