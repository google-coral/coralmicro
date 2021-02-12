#include "libs/base/random.h"

#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.h"
#include <cstdio>

bool GetRandomNumber(void *out, size_t len) {
    caam_handle_t handle;
    handle.jobRing = kCAAM_JobRing0;
    status_t status = CAAM_RNG_GetRandomData(CAAM, &handle, kCAAM_RngStateHandle0, out, len, kCAAM_RngDataAny, NULL);
    return (status == kStatus_Success);
}
