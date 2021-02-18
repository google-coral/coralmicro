#include "libs/base/random.h"
#include "libs/base/tasks_m7.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.h"
#include <cstdio>

namespace valiant {

void Random::StaticTaskMain(void *param) {
    GetSingleton()->TaskMain(param);
}

void Random::TaskMain(void *param) {
    RandomRequest req;
    while (true) {
        if (xQueueReceive(request_queue_, &req, portMAX_DELAY) == pdTRUE) {
            caam_handle_t handle;
            handle.jobRing = kCAAM_JobRing0;
            status_t status = CAAM_RNG_GetRandomData(CAAM, &handle, kCAAM_RngStateHandle0, req.out, req.len, kCAAM_RngDataAny, NULL);
            req.callback(status == kStatus_Success);
        }
    }
}

void Random::Init() {
    request_queue_ = xQueueCreate(4, sizeof(RandomRequest));
    xTaskCreateStatic(
            StaticTaskMain, "random_main", kRandomTaskStackDepth, NULL,
            RANDOM_TASK_PRIORITY, random_task_stack_, &random_task_);
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
    xQueueSend(request_queue_, &req, pdMS_TO_TICKS(200));
    xSemaphoreTake(req_semaphore, pdMS_TO_TICKS(200));
    vSemaphoreDelete(req_semaphore);
    return ret;
}

}  // namespace valiant
