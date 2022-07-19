/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_BASE_RANDOM_H_
#define LIBS_BASE_RANDOM_H_

#include <cstddef>
#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"

namespace coralmicro {

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

}  // namespace coralmicro

#endif  // LIBS_BASE_RANDOM_H_
