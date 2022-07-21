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

#include "libs/base/random.h"

#include <cstddef>
#include <cstdio>
#include <functional>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.h"

namespace coralmicro {
namespace {
struct Response {
  bool success;
};

struct Request {
  void* out;
  size_t len;
  std::function<void(Response)> callback;
};

constexpr char kRandomTaskName[] = "random_task";

class Random : public QueueTask<Request, Response, kRandomTaskName,
                                configMINIMAL_STACK_SIZE * 10,
                                kRandomTaskPriority, /*QueueLength=*/4> {
 public:
  static Random* GetSingleton() {
    static Random random;
    return &random;
  }

  bool Generate(void* out, size_t len) {
    Request req;
    req.out = out;
    req.len = len;
    Response resp = SendRequest(req);
    return resp.success;
  }

 private:
  void RequestHandler(Request* req) override {
    Response resp;
    caam_handle_t handle;
    handle.jobRing = kCAAM_JobRing0;
    status_t status =
        CAAM_RNG_GetRandomData(CAAM, &handle, kCAAM_RngStateHandle0, req->out,
                               req->len, kCAAM_RngDataAny, nullptr);
    DCACHE_InvalidateByRange(reinterpret_cast<uint32_t>(req->out), req->len);
    resp.success = (status == kStatus_Success);
    req->callback(resp);
  }
};

}  // namespace

void RandomInit() { Random::GetSingleton()->Init(); }

bool RandomGenerate(void* buf, size_t size) {
  return Random::GetSingleton()->Generate(buf, size);
}

}  // namespace coralmicro
