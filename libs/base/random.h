#ifndef _LIBS_BASE_RANDOM_H_
#define _LIBS_BASE_RANDOM_H_

#include "libs/base/tasks_m7.h"
#include "libs/base/queue_task.h"
#include <cstddef>
#include <functional>

namespace valiant {

namespace random {

struct Response {
    bool success;
};

struct Request {
    void *out;
    size_t len;
    std::function<void(Response)> callback;
};

}  // namespace random

static constexpr size_t kRandomTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kRandomTaskQueueLength = 4;
extern const char kRandomTaskName[];

class Random : public QueueTask<random::Request, random::Response, kRandomTaskName, kRandomTaskStackDepth, RANDOM_TASK_PRIORITY, kRandomTaskQueueLength> {
  public:
    static Random *GetSingleton() {
        static Random random;
        return &random;
    }
    bool GetRandomNumber(void *out, size_t len);
  private:
    void RequestHandler(random::Request *req) override;
};

}  // namespace valiant

#endif  // _LIBS_BASE_RANDOM_H_
