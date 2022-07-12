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

#include "libs/a71ch/a71ch.h"

#include "libs/base/gpio.h"
#include "third_party/a71ch/hostlib/hostLib/inc/a71ch_api.h"
#include "third_party/a71ch/sss/ex/inc/ex_sss_boot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro::a71ch {

bool Init() {
  coralmicro::GpioSet(coralmicro::Gpio::kCryptoRst, false);
  vTaskDelay(pdMS_TO_TICKS(1));
  coralmicro::GpioSet(coralmicro::Gpio::kCryptoRst, true);

  SE_Connect_Ctx_t sessionCtxt = {0};
  if (auto status = sss_session_open(nullptr, kType_SSS_SE_A71CH, 0,
                                     kSSS_ConnectionType_Plain, &sessionCtxt);
      kStatus_SSS_Success != status) {
    return false;
  }
  return true;
}

std::optional<std::string> GetUId() {
  uint16_t uid_len = A71CH_MODULE_UNIQUE_ID_LEN;
  std::string uid;
  uid.resize(uid_len);
  if (A71_GetUniqueID(reinterpret_cast<U8*>(uid.data()), &uid_len) !=
          SMCOM_OK ||
      uid_len != A71CH_MODULE_UNIQUE_ID_LEN) {
    return std::nullopt;
  }
  return uid;
}

std::optional<std::string> GetRandomBytes(uint8_t num_bytes) {
  std::string random;
  random.resize(num_bytes);
  if (A71_GetRandom(reinterpret_cast<U8*>(random.data()), num_bytes) !=
      SMCOM_OK) {
    return std::nullopt;
  }
  return random;
}

std::optional<std::string> GetEccPublicKey(uint8_t key_idx) {
  // Anything less than 65 will fail, anything more than 65 will just fill the
  // returning key with trailing 0s.
  uint16_t key_len = 65;
  std::string public_ecc_key;
  public_ecc_key.resize(key_len);
  if (A71_GetPublicKeyEccKeyPair(key_idx,
                                 reinterpret_cast<U8*>(public_ecc_key.data()),
                                 &key_len) != SMCOM_OK) {
    return std::nullopt;
  }
  return public_ecc_key;
}

std::optional<std::string> GetSha256(uint8_t* data, uint16_t data_len) {
  // Anything less than 32 will fail, anything more than 32 will just fill the
  // returning key with trailing 0s.
  uint16_t sha_len = 32;
  std::string sha_256;
  sha_256.resize(sha_len);
  if (A71_GetSha256(data, data_len, reinterpret_cast<U8*>(sha_256.data()),
                    &sha_len) != SMCOM_OK) {
    return std::nullopt;
  }
  return sha_256;
}

std::optional<std::string> GetSha256(const std::vector<uint8_t>& data) {
  return GetSha256(const_cast<uint8_t*>(data.data()), data.size());
}

std::optional<std::string> GetEccSignature(uint8_t key_idx, const uint8_t* sha,
                                           uint16_t sha_len) {
  uint16_t signature_len = 256;
  std::string signature;
  signature.resize(signature_len);
  if (A71_EccSign(key_idx, sha, sha_len,
                  reinterpret_cast<U8*>(signature.data()),
                  &signature_len) != SMCOM_OK) {
    return std::nullopt;
  }
  signature.resize(signature_len);  // Resize to the returned size.
  return signature;
}

std::optional<std::string> GetEccSignature(uint8_t key_idx,
                                           const std::vector<uint8_t>& data) {
  return GetEccSignature(key_idx, data.data(), data.size());
}

std::optional<std::string> GetEccSignature(uint8_t key_idx,
                                           const std::string& sha) {
  return GetEccSignature(key_idx, reinterpret_cast<const uint8_t*>(sha.data()),
                         sha.size());
}

bool EccVerify(uint8_t key_idx, const uint8_t* sha, uint16_t sha_len,
               const uint8_t* signature, uint16_t signature_len) {
  if (auto maybe_key = GetEccPublicKey(key_idx); maybe_key.has_value()) {
    const auto& ecc_key = maybe_key.value();
    return EccVerifyWithKey(reinterpret_cast<const uint8_t*>(ecc_key.data()),
                            ecc_key.size(), sha, sha_len, signature,
                            signature_len);
  }
  return false;
}

bool EccVerify(uint8_t key_idx, const std::string& sha,
               const std::string& signature) {
  return EccVerify(
      key_idx, reinterpret_cast<const uint8_t*>(sha.data()), sha.size(),
      reinterpret_cast<const uint8_t*>(signature.data()), signature.size());
}

bool EccVerifyWithKey(const uint8_t* ecc_pub_key, uint16_t key_len,
                      const uint8_t* sha, uint16_t sha_len,
                      const uint8_t* signature, uint16_t signature_len) {
  uint8_t result;
  if (A71_EccVerifyWithKey(ecc_pub_key, key_len, sha, sha_len, signature,
                           signature_len, &result) != SMCOM_OK) {
    return false;
  }
  return result;
}

bool EccVerifyWithKey(const std::string& ecc_key, const std::string& sha,
                      const std::string& signature) {
  return EccVerifyWithKey(
      reinterpret_cast<const uint8_t*>(ecc_key.data()), ecc_key.size(),
      reinterpret_cast<const uint8_t*>(sha.data()), sha.size(),
      reinterpret_cast<const uint8_t*>(signature.data()), signature.size());
}
}  // namespace coralmicro::a71ch
