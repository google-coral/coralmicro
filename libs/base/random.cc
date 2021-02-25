#include "libs/base/random.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.h"
#include <cstdio>

namespace valiant {

constexpr const char kRandomTaskName[] = "random_task";

void Random::MessageHandler(RandomRequest *req) {
    caam_handle_t handle;
    handle.jobRing = kCAAM_JobRing0;
    status_t status = CAAM_RNG_GetRandomData(CAAM, &handle, kCAAM_RngStateHandle0, req->out, req->len, kCAAM_RngDataAny, NULL);
    req->callback(status == kStatus_Success);
}

bool Random::GetRandomNumber(void *out, size_t len) {
    bool ret = false;
    SemaphoreHandle_t req_semaphore = xSemaphoreCreateBinary();
    RandomRequest req = {
        out, len, [req_semaphore, &ret](bool success) {
            ret = success;
            xSemaphoreGive(req_semaphore);
        }
    };
    xQueueSend(message_queue_, &req, pdMS_TO_TICKS(200));
    xSemaphoreTake(req_semaphore, pdMS_TO_TICKS(200));
    vSemaphoreDelete(req_semaphore);
    return ret;
}

}  // namespace valiant
