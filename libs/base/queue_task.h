#ifndef LIBS_BASE_QUEUE_TASK_H_
#define LIBS_BASE_QUEUE_TASK_H_

#include "libs/base/check.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/queue.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coral::micro {

constexpr size_t kDefaultTaskStackDepth = configMINIMAL_STACK_SIZE;
template <typename Request, typename Response, const char* Name,
          size_t StackDepth, UBaseType_t Priority, UBaseType_t QueueLength>
class QueueTask {
   public:
    virtual ~QueueTask() = default;

    // Initialization function for the QueueTask.
    // If overridden, implementors should ensure that the super method is still
    // called. Initialized any needed data here, but beware that the FreeRTOS
    // scheduler may not yet be running.
    virtual void Init() {
        request_queue_ = xQueueCreate(QueueLength, sizeof(Request));
        CHECK(request_queue_);
        CHECK(xTaskCreateStatic(StaticTaskMain, Name, StackDepth, this,
                                Priority, task_stack_, &task_));
    }

   protected:
    Response SendRequest(Request& req) {
        Response resp;
        SemaphoreHandle_t sem = xSemaphoreCreateBinary();
        req.callback = [sem, &resp](Response cb_resp) {
            resp = cb_resp;
            CHECK(xSemaphoreGive(sem) == pdTRUE);
        };
        CHECK(xQueueSend(request_queue_, &req, pdMS_TO_TICKS(200)) == pdTRUE);
        CHECK(xSemaphoreTake(sem, pdMS_TO_TICKS(200)) == pdTRUE);
        vSemaphoreDelete(sem);
        return resp;
    }

    void SendRequestAsync(Request& req) {
        CHECK(xQueueSend(request_queue_, &req, pdMS_TO_TICKS(200)) == pdTRUE);
    }

    StaticTask_t task_;
    StackType_t task_stack_[StackDepth];
    QueueHandle_t request_queue_;

   private:
    static void StaticTaskMain(void* param) {
        static_cast<QueueTask*>(param)->TaskMain();
    }

    // Main QueueTask FreeRTOS task function.
    // Calls implementation-specific initialization, and then starts receiving
    // from the queue. This will block and yield the CPU if no messages are
    // available. When a message is received, the implementation-specific
    // handler is invoked.
    [[noreturn]] void TaskMain() {
        TaskInit();

        Request msg;
        while (true) {
            if (xQueueReceive(request_queue_, &msg, portMAX_DELAY) == pdTRUE) {
                RequestHandler(&msg);
            }
        }
    }

    // Implementation-specific hook for initialization at the start of the task
    // function. FreeRTOS scheduler is running at this point, so it's okay to
    // use functions that may require it.
    virtual void TaskInit(){};

    // Implementation-specific handler for messages coming from the queue.
    virtual void RequestHandler(Request* msg) = 0;
};

}  // namespace coral::micro

#endif  // LIBS_BASE_QUEUE_TASK_H_
