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

#include <cstdio>

#include "libs/a71ch/a71ch.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/strings.h"
#include "third_party/a71ch/hostlib/hostLib/inc/a71ch_api.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

extern "C" void app_main(void* param) {
  vTaskDelay(pdMS_TO_TICKS(1000));
  // Initializes the A71 chip.
  if (!coralmicro::a71ch::Init()) {
    printf("Failed to initializes the a71ch chip\r\n");
    vTaskSuspend(nullptr);
  }

  // Get the A71's unique id.
  if (auto a71ch_uid = coralmicro::a71ch::GetUId(); a71ch_uid.has_value()) {
    printf("A71 Unique ID: %s\r\n",
           coralmicro::StrToHex(a71ch_uid.value()).c_str());
  } else {
    printf("Failed to retrieve A71's unique ID\r\n");
    vTaskSuspend(nullptr);
  }

  // Get random bytes from the A71.
  for (uint8_t random_len = 8; random_len <= 64; random_len <<= 1) {
    if (auto random_bytes = coralmicro::a71ch::GetRandomBytes(random_len);
        random_bytes.has_value()) {
      printf("A71 Random %d Bytes: %s\r\n", random_len,
             coralmicro::StrToHex(random_bytes.value()).c_str());
    } else {
      printf("Failed to get random bytes from A71\r\n");
      vTaskSuspend(nullptr);
    }
  }

  // The storage index of the ecc key pair to sign and verify.
  constexpr uint8_t kKeyIdx = 0;

  // Get the public ECC key at index 0 from the A71 chip.
  auto maybe_ecc_pub_key = coralmicro::a71ch::GetEccPublicKey(kKeyIdx);
  if (maybe_ecc_pub_key.has_value()) {
    printf("A71 ECC public key %d: %s\r\n", kKeyIdx,
           coralmicro::StrToHex(maybe_ecc_pub_key.value()).c_str());
  } else {
    printf("Failed to get A71 ECC public key %d\r\n", kKeyIdx);
    vTaskSuspend(nullptr);
  }

  // Get the sha256 and ecc signature for this model file.
  std::vector<uint8_t> model;
  constexpr char kModelPath[] = "/models/testconv1-edgetpu.tflite";
  if (!coralmicro::LfsReadFile(kModelPath, &model)) {
    printf("%s missing\r\n", kModelPath);
    vTaskSuspend(nullptr);
  }
  auto maybe_sha = coralmicro::a71ch::GetSha256(model);
  if (maybe_sha.has_value()) {
    printf("testconv1-edgetpu.tflite sha: %s\r\n",
           coralmicro::StrToHex(maybe_sha.value()).c_str());
  } else {
    printf("failed to generate sha256 for testconv1-edgetpu.tflite\r\n");
    vTaskSuspend(nullptr);
  }

  // Get signature for this model with the key public key in index 0.
  const auto& sha = maybe_sha.value();
  auto maybe_signature = coralmicro::a71ch::GetEccSignature(kKeyIdx, sha);
  if (maybe_signature.has_value()) {
    printf("Signature: %s\r\n",
           coralmicro::StrToHex(maybe_signature.value()).c_str());
  } else {
    printf("failed to get ecc signature\r\n");
    vTaskSuspend(nullptr);
  }

  // Verifying the signature using the key storage index.
  const auto& signature = maybe_signature.value();
  printf("EccVerify: %s\r\n",
         coralmicro::a71ch::EccVerify(kKeyIdx, sha, signature) ? "success"
                                                               : "fail");

  // Verifying the signature using the key.
  const auto& ecc_pub_key = maybe_ecc_pub_key.value();
  printf("EccVerifyWithKey: %s\r\n",
         coralmicro::a71ch::EccVerifyWithKey(ecc_pub_key, sha, signature)
             ? "success"
             : "fail");

  printf(
      "\r\nSee examples/security/README.md for instructions to verify "
      "manually.\r\n");
  vTaskSuspend(nullptr);
}
