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

// Runs various cryptographic tasks using the A71CH secure element.

#include "Arduino.h"
#include "libs/a71ch/a71ch.h"
#include "libs/base/filesystem.h"
#include "libs/base/strings.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  delay(1000);
  Serial.println("Arduino Security!");

  // Initializes the A71 chip.
  if (!coralmicro::A71ChInit()) {
    Serial.println("Failed to initializes the a71ch chip");
    return;
  }

  // Get the A71's unique id.
  if (auto a71ch_uid = coralmicro::A71ChGetUId(); a71ch_uid.has_value()) {
    Serial.print("A71 Unique ID: ");
    Serial.println(coralmicro::StrToHex(a71ch_uid.value()).c_str());
  } else {
    Serial.println("Failed to retrieve A71's unique ID");
    return;
  }

  // The storage index of the ecc key pair to sign and verify.
  constexpr uint8_t kKeyIdx = 0;

  // Get the public ECC key at index 0 from the A71 chip.
  auto maybe_ecc_pub_key = coralmicro::A71ChGetEccPublicKey(kKeyIdx);
  if (maybe_ecc_pub_key.has_value()) {
    Serial.print("A71 ECC public key: ");
    Serial.println(coralmicro::StrToHex(maybe_ecc_pub_key.value()).c_str());
  } else {
    Serial.println("Failed to get A71 ECC public key");
    return;
  }

  // Get the sha256 and ecc signature for this model file.
  std::vector<uint8_t> model;
  constexpr char kModelPath[] = "/models/testconv1-edgetpu.tflite";
  if (!coralmicro::LfsReadFile(kModelPath, &model)) {
    Serial.println("/models/testconv1-edgetpu.tflite missing");
    return;
  }
  auto maybe_sha = coralmicro::A71ChGetSha256(model);
  if (maybe_sha.has_value()) {
    Serial.print("testconv1-edgetpu.tflite sha");
    Serial.println(coralmicro::StrToHex(maybe_sha.value()).c_str());
  } else {
    Serial.println("failed to generate sha256 for testconv1-edgetpu.tflite");
    return;
  }

  // Get signature for this model with the key public key in index 0.
  const auto& sha = maybe_sha.value();
  auto maybe_signature = coralmicro::A71ChGetEccSignature(kKeyIdx, sha);
  if (maybe_signature.has_value()) {
    Serial.print("Signature: ");
    Serial.println(coralmicro::StrToHex(maybe_signature.value()).c_str());
  } else {
    Serial.println("failed to get ecc signature");
    return;
  }

  // Verifying the signature using the key storage index.
  const auto& signature = maybe_signature.value();
  Serial.print("EccVerify: ");
  Serial.println(coralmicro::A71ChEccVerify(kKeyIdx, sha, signature) ? "success"
                                                                     : "fail");

  // Verifying the signature using the key.
  const auto& ecc_pub_key = maybe_ecc_pub_key.value();
  Serial.print("EccVerifyWithKey: ");
  Serial.println(coralmicro::A71ChEccVerifyWithKey(ecc_pub_key, sha, signature)
                     ? "success"
                     : "fail");
}

void loop() {}
