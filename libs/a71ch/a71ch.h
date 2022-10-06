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

#ifndef LIBS_A71CH_A71CH_H_
#define LIBS_A71CH_A71CH_H_

#include <optional>
#include <string>
#include <vector>

namespace coralmicro {

// Initializes the a71ch module.
//
// You must call this before using any of the following a71ch helper functions
// or the direct a71ch APIs (from `third_party/a71ch/`). You must also call this
// before using SSL features in Mbed TLS
// (from `third_party/nxp/rt1176-sdk/middleware/mbedtls/`) or before requesting
// an HTTPS URL with Curl (from `libs/curl/curl.h`).
bool A71ChInit();

// Gets the uid of the a71ch module.
//
// @return The unique id of this a71ch module or std::nullopt.
std::optional<std::string> A71ChGetUId();

// Gets the random bytes from the A71.
//
// @param num_bytes The number of random bytes.
// @return The random bytes or std::nullopt on failure.
std::optional<std::string> A71ChGetRandomBytes(uint8_t num_bytes);

// Gets a public Elliptical Curve key from the A71.
//
// @param key_idx The key storage index to obtain the key. Note: The a71ch chip
// can only hold up to 4 key pairs, so this function will fail if key_idx > 3.
// See the manual referenced above for more info.
// @return The public ECC key in index key_idx or std::nullopt if failed to
// retrieve.
std::optional<std::string> A71ChGetEccPublicKey(uint8_t key_idx);

// Gets the sha256 hash for a buffer of data.
//
// @param data The buffer to the raw data to generate the hash.
// @param data_len The length of the data buffer.
// @return The sha256 hash for data or std::nullopt on failure.
std::optional<std::string> A71ChGetSha256(uint8_t* data, uint16_t data_len);

// Gets the sha256 hash for a vector of data.
//
// @param data The raw data to generate the hash.
// @return The sha256 hash for data or std::nullopt on failure.
std::optional<std::string> A71ChGetSha256(const std::vector<uint8_t>& data);

// Gets the Elliptical Curve signature for a hash.
//
// @param key_idx The key storage index of the public key to sign this hash.
// @param sha The buffer containing the sha256 hash of some data to sign.
// @param sha_len The length of the sha.
// @return The Ecc Signature or std::nullopt on failure.
std::optional<std::string> A71ChGetEccSignature(uint8_t key_idx,
                                                const uint8_t* sha,
                                                uint16_t sha_len);

// Gets the Elliptical Curve signature for a hash.
//
// @param key_idx The key storage index of the public key to sign this hash.
// @param sha The sha256 hash for some data to sign.
// @return The Ecc Signature or std::nullopt on failure.
std::optional<std::string> A71ChGetEccSignature(
    uint8_t key_idx, const std::vector<uint8_t>& data);

// Gets the Elliptical Curve signature for a hash.
//
// @param key_idx The key storage index of the public key to sign this hash.
// @param sha The string of sha256 hash for some data to sign.
// @return The Ecc Signature or std::nullopt on failure.
std::optional<std::string> A71ChGetEccSignature(uint8_t key_idx,
                                                const std::string& sha);

// Verify whether signature is the signature of sha using ecc_key.
//
// @param key_idx The storage index of the ecc public key that was used to sign
// the sha.
// @param sha The sha256 hash to verify.
// @param sha_len The length of sha.
// @param signature The signature to verify.
// @param signature_len The length of the signature.
// @return true if verify success, else false.
bool A71ChEccVerify(uint8_t key_idx, const uint8_t* sha, uint16_t sha_len,
                    const uint8_t* signature, uint16_t signature_len);

// Verify whether signature is the signature of sha using ecc_key.
//
// @param key_idx The storage index of the ecc public key that was used to sign
// the sha.
// @param sha The sha256 hash to verify.
// @param signature The signature to verify.
// @return true if verify success, else false.
bool A71ChEccVerify(uint8_t key_idx, const std::string& sha,
                    const std::string& signature);

// Verify whether signature is the signature of sha using ecc_key.
//
// @param ecc_pub_key The Elliptical Curve public key to use in order to verify.
// Note: This must be the publickey from the same storage index that was used to
// sign the sha.
// @param key_len The length of ecc_pub_key.
// @param sha The sha256 hash to verify.
// @param sha_len The length of sha.
// @param signature The signature to verify.
// @param signature_len The length of the signature.
// @return true if verify success, else false.
bool A71ChEccVerifyWithKey(const uint8_t* ecc_pub_key, uint16_t key_len,
                           const uint8_t* sha, uint16_t sha_len,
                           const uint8_t* signature, uint16_t signature_len);

// Verify whether signature is the signature of sha using ecc_key.
//
// @param ecc_pub_key The Elliptical Curve public key to use in order to verify.
// Note: This must be the publickey from the same storage index that was used to
// sign the sha.
// @param sha The sha256 hash to verify.
// @param signature The signature to verify.
// @return true if verify success, else false.
bool A71ChEccVerifyWithKey(const std::string& ecc_pub_key,
                           const std::string& sha,
                           const std::string& signature);
}  // namespace coralmicro

#endif  // LIBS_A71CH_A71CH_H_
