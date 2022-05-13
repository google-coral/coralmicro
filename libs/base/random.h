#ifndef _LIBS_BASE_RANDOM_H_
#define _LIBS_BASE_RANDOM_H_

#include <cstddef>
#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"

namespace coral::micro {

namespace random {

struct Response {
    bool success;
};

struct Request {
    void* out;
    size_t len;
    std::function<void(Response)> callback;
};

}  // namespace random

inline constexpr char kRandomTaskName[] = "random_task";

class Random : public QueueTask<random::Request, random::Response,
                                kRandomTaskName, configMINIMAL_STACK_SIZE * 10,
                                RANDOM_TASK_PRIORITY, /*QueueLength=*/4> {
   public:
    static Random* GetSingleton() {
        static Random random;
        return &random;
    }
    bool GetRandomNumber(void* out, size_t len);

   private:
    void RequestHandler(random::Request* req) override;
};

}  // namespace coral::micro

#endif  // _LIBS_BASE_RANDOM_H_
