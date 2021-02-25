#ifndef _LIBS_BASE_QUEUE_TASK_H_
#define _LIBS_BASE_QUEUE_TASK_H_

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/queue.h"
#include "third_party/freertos_kernel/include/task.h"

namespace valiant {

constexpr size_t kDefaultTaskStackDepth = configMINIMAL_STACK_SIZE;
template <typename Message, const char *Name, size_t StackDepth, UBaseType_t Priority, UBaseType_t QueueLength>
class QueueTask {
  public:
    // Initialization function for the QueueTask.
    // If overridden, implementors should ensure that the super method is still called.
    // Initialized any needed data here, but beware that the FreeRTOS scheduler may not yet be running.
    virtual void Init() {
        message_queue_ = xQueueCreate(QueueLength, sizeof(Message));
        xTaskCreateStatic(
                StaticTaskMain, Name, stack_depth_, this,
                Priority, task_stack_, &task_);
    }
  protected:
    const size_t stack_depth_ = StackDepth;
    StaticTask_t task_;
    StackType_t task_stack_[StackDepth];
    QueueHandle_t message_queue_;
  private:
    static void StaticTaskMain(void *param) {
        ((QueueTask*)param)->TaskMain();
    }

    // Main QueueTask FreeRTOS task function.
    // Calls implementation-specific initialization, and then starts receiving from the queue.
    // This will block and yield the CPU if no messages are available.
    // When a message is received, the implementation-specific handler is invoked.
    void TaskMain() {
        TaskInit();

        Message msg;
        while (true) {
            if (xQueueReceive(message_queue_, &msg, portMAX_DELAY) == pdTRUE) {
                MessageHandler(&msg);
            }
        }
    }

    // Implementation-specific hook for initialization at the start of the task function.
    // FreeRTOS scheduler is running at this point, so it's okay to use functions that may require it.
    virtual void TaskInit() {};

    // Implementation-specific handler for messages coming from the queue.
    virtual void MessageHandler(Message* msg) = 0;
};

}  // namespace valiant

#endif  // _LIBS_BASE_QUEUE_TASK_H_
