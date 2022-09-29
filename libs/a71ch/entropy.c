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

#include "third_party/nxp/rt1176-sdk/middleware/mbedtls/include/mbedtls/entropy.h"

#include <stddef.h>

#include "third_party/a71ch-crypto-support/hostlib/hostLib/inc/a71ch_api.h"

// data is allowed to be NULL
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len,
                          size_t *olen) {
  U16 ret = A71_GetRandom(output, len);
  if (ret == SMCOM_OK) {
    if (olen) {
      *olen = len;
    }
    return 0;
  }
  return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
}
