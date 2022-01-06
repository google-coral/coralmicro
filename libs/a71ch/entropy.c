#include "third_party/a71ch/hostlib/hostLib/inc/a71ch_api.h"
#include "third_party/nxp/rt1176-sdk/middleware/mbedtls/include/mbedtls/entropy.h"
#include <stddef.h>

// data is allowed to be NULL
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
    U16 ret = A71_GetRandom(output, len);
    if (ret == SMCOM_OK) {
        if (olen) {
            *olen = len;
        }
        return 0;
    }
    return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
}