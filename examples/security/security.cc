// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "libs/a71ch/a71ch.h"

#include <cstdio>

#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "third_party/a71ch/hostlib/hostLib/inc/a71ch_api.h"
#include "third_party/a71ch/sss/ex/inc/ex_sss_boot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

extern "C" [[noreturn]] void app_main(void* param) {
    coral::micro::a71ch::Init();
    uint8_t uid[A71CH_MODULE_UNIQUE_ID_LEN];
    uint16_t uidLen = A71CH_MODULE_UNIQUE_ID_LEN;
    uint16_t ret = A71_GetUniqueID(uid, &uidLen);

    if (ret == SMCOM_OK) {
        printf("A71 Unique ID: ");
        for (int i = 0; i < uidLen; ++i) {
            printf("%02x", uid[i]);
        }
        printf("\r\n");
    } else {
        printf("Failed to retrieve A71's unique ID\r\n");
        vTaskSuspend(nullptr);
    }

    constexpr uint8_t randomLen = 16;
    uint8_t random[randomLen];
    ret = A71_GetRandom(random, randomLen);
    if (ret == SMCOM_OK) {
        printf("A71 Random bytes: ");
        for (int i = 0; i < randomLen; ++i) {
            printf("%02x", random[i]);
        }
        printf("\r\n");
    } else {
        printf("Failed to get random data from A71\r\n");
        vTaskSuspend(nullptr);
    }

    uint16_t publicKeyLen = 65;
    uint8_t publicKey[publicKeyLen];
    ret = A71_GetPublicKeyEccKeyPair(0, publicKey, &publicKeyLen);
    if (ret == SMCOM_OK) {
        printf("A71 public key 0: ");
        for (int i = 0; i < publicKeyLen; ++i) {
            printf("%02x", publicKey[i]);
        }
        printf("\r\n");
    } else {
        printf("couldn't get keypair\r\n");
        vTaskSuspend(nullptr);
    }

    std::vector<uint8_t> model;
    constexpr char kModelPath[] = "/models/testconv1-edgetpu.tflite";
    if (!coral::micro::filesystem::ReadFile(kModelPath, &model)) {
        printf("%s missing\r\n", kModelPath);
        vTaskSuspend(nullptr);
    }

    uint16_t shaLen = 32;
    uint8_t sha[shaLen];
    ret = A71_GetSha256(model.data(), model.size(), sha, &shaLen);
    if (ret == SMCOM_OK) {
        printf("SHA256: ");
        for (int i = 0; i < shaLen; ++i) {
            printf("%02x", sha[i]);
        }
        printf("\r\n");
    } else {
        printf("Failed to sha256sum\r\n");
        vTaskSuspend(nullptr);
    }

    uint16_t signature_len = 256;
    uint8_t signature[signature_len];
    ret = A71_EccSign(0, sha, shaLen, signature, &signature_len);
    if (ret == SMCOM_OK) {
        printf("Signature: ");
        for (int i = 0; i < signature_len; ++i) {
            printf("%02x", signature[i]);
        }
        printf("\r\n");
    } else {
        printf("Failed to sign\r\n");
        vTaskSuspend(nullptr);
    }
    vTaskSuspend(nullptr);
}
