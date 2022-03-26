#include "libs/a71ch/a71ch.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "third_party/a71ch/hostlib/hostLib/inc/a71ch_api.h"
#include "third_party/a71ch/sss/ex/inc/ex_sss_boot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include <cstdio>

extern "C" [[noreturn]] void app_main(void* param) {
    valiant::a71ch::Init();
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
        vTaskSuspend(NULL);
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
        vTaskSuspend(NULL);
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
        vTaskSuspend(NULL);
    }

    std::vector<uint8_t> model;
    constexpr const char kModelPath[] = "/models/testconv1-edgetpu.tflite";
    if (!valiant::filesystem::ReadFile(kModelPath, &model)) {
        printf("%s missing\r\n", kModelPath);
        vTaskSuspend(NULL);
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
        vTaskSuspend(NULL);
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
        vTaskSuspend(NULL);
    }
    vTaskSuspend(NULL);
}
